#include <stddef.h>
#include <limits.h>
#include <assert.h>
#include <string.h> // for memcpy
#include "compiler.h"
#include "exception.h"
#include "graph.h"
#include "iterator.h"
#include "property.h"
#include "TransactionImpl.h"
#include "GraphImpl.h"
#include "arch.h"

using namespace Jarvis;

namespace Jarvis {
#pragma pack(push, 1)
    struct PropertyRef::BlobRef {
        void *value;
        uint32_t size;
    };
#pragma pack(pop)

    class PropertyListIteratorImpl : public PropertyIteratorImplIntf {
        PropertyRef _cur;
    public:
        PropertyListIteratorImpl(const PropertyList *list)
            : _cur(list)
            { _cur.skip_to_next(); }

        operator bool() const { return _cur.not_done(); }
        const PropertyRef &operator*() const { return _cur; }
        const PropertyRef *operator->() const { return &_cur; }
        PropertyRef &operator*() { return _cur; }
        PropertyRef *operator->() { return &_cur; }
        bool next() { return _cur.next(); }
    };


    // The PropertySpace class contains a space requirement,
    // determined from the type and value of the property to
    // be stored, a PropertyRef, which (if set) refers to
    // sufficient unused space in the property list, and the
    // size of the space found.
    // The _new_chunk flag indicates whether the space referred to is
    // in a new chunk, and thus doesn't need to be logged.
    class PropertyList::PropertySpace {
        unsigned _req;
        unsigned _size;
        PropertyRef _pos;
        bool _new_chunk;

    public:
        PropertySpace(unsigned r) : _req(r), _size(0), _new_chunk(false) { }
        const PropertyRef &pos() const { return _pos; }
        unsigned req() const { return _req; }
        unsigned size() const { return _size; }
        void set_new() { _new_chunk = true; }
        void set_pos(const PropertyRef &p, unsigned sz) { _pos = p; _size = sz; }
        void set_property(StringID id, const Property &,
                          TransactionImpl *, Allocator &);
    };
};


void PropertyList::init(size_t size)
{
    // Size must be at least 10: 1 byte for the chunk size
    // and 9 bytes for a link.
    assert(size >= 10 && size <= 256);

    // This is called only for a newly allocated property chunk,
    // so no logging is required. The flush is done by the caller.
    _chunk0[0] = uint8_t(size - 1);
    _chunk0[1] = PropertyRef::p_end;
}

bool PropertyList::check_property(StringID id, Property &result) const
{
    PropertyRef p;

    if (!find_property(id, p))
        return false;

    result = p.get_value();
    return true;
}

Property PropertyList::get_property(StringID id) const
{
    PropertyRef p;

    if (!find_property(id, p))
        throw Exception(property_not_found);

    return p.get_value();
}

PropertyIterator PropertyList::get_properties() const
{
    return PropertyIterator(new PropertyListIteratorImpl(this));
}

void PropertyList::set_property(StringID id, const Property &new_value,
                                Property &old_value)
{
    TransactionImpl *tx = TransactionImpl::get_tx();
    Allocator &allocator = tx->get_db()->allocator();
    PropertyRef pos;
    PropertySpace space(get_space(new_value) + 3);

    if (find_property(id, pos, &space)) {
        old_value = Property(pos);
        pos.free(tx);
    }

    if (space.size() == 0) {
        if (!find_space(space, pos)) {
            add_chunk(pos, tx, allocator);
            space.set_pos(pos, pos.free_space());
            space.set_new();
        }
    }

    space.set_property(id, new_value, tx, allocator);
}

void PropertyList::remove_property(StringID id)
{
    PropertyRef p;
    if (find_property(id, p))
        p.free(TransactionImpl::get_tx());
}


// Search the property list for the specified property id.
// If it is found, return a reference to the property in r.
// If the property is not found, set r to the end of the list.
// If the space parameter is non-null, also look for space
// that matches the space requirement.
bool PropertyList::find_property(StringID id, PropertyRef &r,
                                 PropertySpace *space) const
{
    PropertyRef p(this);
    while (p.not_done()) {
        switch (p.ptype()) {
            case PropertyRef::p_link:
                p.follow_link();
                continue;
            case PropertyRef::p_unused: {
                if (space != NULL) {
                    unsigned free_space = p.free_space();
                    if (free_space >= space->req()) {
                        // Look for exact fit. Otherwise, use first fit.
                        bool exact = free_space == space->req();
                        if (space->size() == 0 || exact) {
                            space->set_pos(p, free_space);
                            // If we got an exact fit, quit looking.
                            if (exact)
                                space = NULL;
                        }
                    }
                }
                break;
            }
            default:
                if (p.id() == id) {
                    r = p;
                    return true;
                }
                break;
        }
        p.skip();
    }

    r = p;
    return false;
}


// Search for space suitable to meet the specified space requirement.
// Start the search at the position specified.
// Set the pos and size members of the space parameter to the space found.
// If no suitable space is found, return false.
bool PropertyList::find_space(PropertySpace &space, PropertyRef &start) const
{
    PropertyRef p = start;
    while (p.not_done()) {
        switch (p.ptype()) {
            case PropertyRef::p_link:
                p.follow_link();
                continue;
            case PropertyRef::p_unused: {
                unsigned free_space = p.free_space();
                if (free_space >= space.req()) {
                    space.set_pos(p, free_space);
                    return true;
                }
                break;
            }
            default:
                break;
        }
        p.skip();
    }

    // We're at p_end.
    unsigned free_space = p.free_space();
    if (free_space >= space.req()) {
        space.set_pos(p, free_space);
        return true;
    }

    start = p;
    return false;
}


// Allocate and link a new property chunk.
// The parameter 'end' refers to the current end of the property list
// and is changed to refer to the new end after a chunk is added.
void PropertyList::add_chunk(PropertyRef &end,
                             TransactionImpl *tx, Allocator &allocator)
{
    assert(!end.not_done());

    PropertyList *p_chunk = static_cast<PropertyList *>(allocator.alloc(PropertyList::chunk_size));
    p_chunk->init(PropertyList::chunk_size);

    PropertyRef p = end;
    end = PropertyRef(p_chunk);

    // If there isn't space for the link in the current chunk,
    // copy some properties from this chunk to the new one.
    // Make_space sets p to the beginning of the space freed up
    // for the link, and sets end to the beginning of free space
    // in the new chunk.
    if (p.free_space() < sizeof (PropertyList *) + 1)
        p.make_space(end);

    p.set_link(p_chunk, tx);
}


// Make_space is called when we need to add a link from the
// current chunk to a new chunk and there isn't space for a
// link in the current chunk.
// The parameter q refers to the beginning of the new chunk.
// Make_space moves some properties from this chunk to the
// new one. It sets *this to refer the beginning of the space
// freed up for the link, and sets q to the beginning of free
// space in the new chunk.
void PropertyRef::make_space(PropertyRef &q)
{
    PropertyRef p;
    p._chunk = _chunk;
    p._offset = 1;
    while (p._offset + p.size() < chunk_end() - sizeof (PropertyList *))
        p.skip();

    *this = p;

    while (p.not_done()) {
        if (p.ptype() != p_unused) {
            q.copy(p);
            q.skip();
        }
        p.skip();
    }
    q.type_size() = p_end;
}

inline void PropertyRef::copy(const PropertyRef &p)
{
    memcpy(&_chunk[_offset], &p._chunk[p._offset], p.size() + 1);
}


// Determine how much free space is available at the referenced position.
unsigned PropertyRef::free_space() const
{
    PropertyRef p = *this;
    unsigned end = chunk_size();
    while (p.not_done()) {
        if (p.ptype() != p_unused) {
            end = p._offset;
            break;
        }
        p.skip();
    }
    unsigned size = end - _offset;
    return size;
}


// Determine the minimum number of bytes required to store
// the specified value.
static unsigned get_int_len(long long v)
{
    if (v < 0) v = -v - 1;
    if (v == 0) return 1;
    return (bsr(v) + 1) / CHAR_BIT + 1;
}


// Determine the number of bytes required to store the property value.
unsigned PropertyList::get_space(const Property &p)
{
    switch (p.type()) {
        case t_novalue: return 0;
        case t_boolean: return 0;
        case t_integer: return get_int_len(p.int_value());
        case t_string: {
            size_t len = p.string_value().length();
            if (len <= 13) return len;
            return sizeof (PropertyRef::BlobRef);
        }
        case t_float: return sizeof (double);
        case t_time: return sizeof (Time);
        case t_blob: return sizeof (PropertyRef::BlobRef);
        default: assert(0); return 0;
    }
}


// Store the property in the space referred to by _pos.
// The caller should have found suitable space.
// The asserts verify that this was done correctly.
void PropertyList::PropertySpace::set_property(StringID id, const Property &p,
                                               TransactionImpl *tx,
                                               Allocator &allocator)
{
    assert(_req >= 3);
    assert(_size >= _req);
    assert(_pos.free_space() >= _size);

    unsigned log_size = _size > _req ? _req + 1 : _req;

    if (!_new_chunk)
        tx->log(&_pos._chunk[_pos._offset], log_size);

    if (_size > _req)
        _pos.set_size(_size, _req);
    _pos.set_value(p, _req, allocator);
    _pos.set_id(id);

    if (_new_chunk) {
        // Flush from the beginning of the chunk because the
        // chunk initialization code depends on us doing it
        // (to avoid redundant flushes).
        TransactionImpl::flush_range(_pos._chunk, _pos._offset + log_size);
    }
}


void PropertyRef::free(TransactionImpl *tx)
{
    if (ptype() == p_string_ptr || ptype() == p_blob) {
        Allocator &allocator = tx->get_db()->allocator();
        BlobRef *v = (BlobRef *)val();
        allocator.free(v->value, v->size);
    }
    PropertyRef next(*this, size() + 1);
    if (!next.not_done())
        tx->write(&type_size(), uint8_t(p_end));
    else
        tx->write(&type_size(), uint8_t(size() << 4 | p_unused));
}

// Carve out space for a new property in the property list.
// If the new property is being added at the end,
// mark the following space as the new end,
// otherwise, mark the following space as unused.
void PropertyRef::set_size(unsigned old_size, unsigned new_size)
{
    PropertyRef next(*this, new_size);
    if (old_size == chunk_size() - _offset)
        next.type_size() = p_end;
    else {
        PropertyRef p = *this;
        unsigned unused_size = 0;
        while ((unused_size += p.size() + 1) < new_size)
            p.skip();
        if (unused_size > new_size)
            next.type_size() = (unused_size - new_size - 1) << 4 | p_unused;
    }
}


// Advance the reference to the next property, returning false if
// there isn't one.
bool PropertyRef::skip_to_next()
{
    while (not_done()) {
        switch (ptype()) {
            case p_link:
                follow_link();
                continue;
            case p_unused:
                break;
            default:
                return true;
        }
        skip();
    }
    return false;
}


// Store a link to the next chunk in the space referred to.
// If there is more space than required,
// put the excess prior to the link, marked as unused.
void PropertyRef::set_link(PropertyList *p_chunk, TransactionImpl *tx)
{
    unsigned remaining_size = chunk_size() - _offset;
    unsigned req = sizeof p_chunk + 1;
    assert(remaining_size >= req);
    unsigned unused_size = remaining_size - req;
    assert(unused_size < 16);
    tx->log(&_chunk[_offset], remaining_size);
    if (unused_size > 0) {
        type_size() = uint8_t((unused_size - 1) << 4) | p_unused;
        _offset += unused_size;
    }
    type_size() = p_link;
    link() = p_chunk;
}


// Store the property value in the space referred to.
void PropertyRef::set_value(const Property &p, unsigned size,
                            Allocator &allocator)
{
    assert(_offset <= chunk_end());
    unsigned type;
    switch (p.type()) {
        case t_novalue:
            type = p_novalue;
            break;
        case t_boolean:
            type = p.bool_value() ? p_boolean_true : p_boolean_false;
            break;
        case t_integer: {
            long long v = p.int_value();
            assert(size == get_int_len(v) + 3);
            memcpy(val(), &v, size - 3);
            type = p_integer;
            break;
        }
        case t_string: {
            const std::string &value = p.string_value();
            size_t len = value.length();
            if (len <= 13) {
                if (len > 0)
                    memcpy(val(), value.data(), len);
                type = p_string;
            }
            else {
                set_blob(value.data(), len, allocator);
                type = p_string_ptr;
            }
            break;
        }
        case t_float:
            *(double *)val() = p.float_value();
            type = p_float;
            break;
        case t_time:
            *(Time *)val() = p.time_value();
            type = p_time;
            break;
        case t_blob: {
            Property::blob_t value = p.blob_value();
            set_blob(value.value, value.size, allocator);
            type = p_blob;
            break;
        }
        default:
            type = 0;
            assert(0);
    }
    type_size() = uint8_t((size - 1) << 4 | type);
}

void PropertyRef::set_blob(const void *value, std::size_t size,
                           Allocator &allocator)
{
    if (size > UINT_MAX) throw Exception(not_implemented);
    void *p = allocator.alloc(size);
    memcpy(p, value, size);
    TransactionImpl::flush_range(p, size);
    BlobRef *v = (BlobRef *)val();
    v->value = p;
    v->size = uint32_t(size);
}


// The following functions return the value of the referenced property,
// throwing an exception if the type of the property doesn't match that
// requested.
bool PropertyRef::bool_value() const
{
    switch (ptype()) {
        case p_boolean_false: return false;
        case p_boolean_true: return true;
    }
    throw Exception(property_type);
}

long long PropertyRef::int_value() const
{
    if (ptype() == p_integer) {
        unsigned sz = size() - 2;
        unsigned shift = unsigned(sizeof (long long)) - sz;
        long long v = *(long long *)(val() - shift);
        if (sz < sizeof (long long))
            v >>= CHAR_BIT * shift;
        return v;
    }
    throw Exception(property_type);
}

std::string PropertyRef::string_value() const
{
    switch (ptype()) {
        case p_string: {
            unsigned sz = size() - 2;
            if (sz > 0)
                return std::string((const char *)val(), sz);
            else
                return "";
        }
        case p_string_ptr: {
            BlobRef *v = (BlobRef *)val();
            return std::string((const char *)v->value, v->size);
        }
    }
    throw Exception(property_type);
}

double PropertyRef::float_value() const
{
    if (ptype() == p_float)
        return *(double *)val();
    throw Exception(property_type);
}

Time PropertyRef::time_value() const
{
    if (ptype() == p_time)
        return *(Time *)val();
    throw Exception(property_type);
}

Property::blob_t PropertyRef::blob_value() const
{
    if (ptype() == p_blob) {
        BlobRef *v = (BlobRef *)val();
        return Property::blob_t(v->value, v->size);
    }
    throw Exception(property_type);
}


// Return the value of the referenced property.
Property PropertyRef::get_value() const
{
    switch (ptype()) {
        case p_novalue: return Property();
        case p_boolean_false: return Property(false);
        case p_boolean_true: return Property(true);
        case p_integer: return Property(int_value());
        case p_string: return Property(string_value());
        case p_string_ptr: return Property(string_value());
        case p_float: return Property(float_value());
        case p_time: return Property(time_value());
        case p_blob: return Property(blob_value());
        default: assert(0); return Property();
    }
}
