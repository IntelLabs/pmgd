#include <stddef.h>
#include <limits.h>
#include <assert.h>
#include <string.h> // for memcpy
#include "exception.h"
#include "graph.h"
#include "iterator.h"
#include "property.h"
#include "TransactionImpl.h"
#include "GraphImpl.h"
#include "arch.h"

using namespace Jarvis;

namespace Jarvis {
    struct __attribute((packed)) PropertyRef::BlobRef {
        const void *value;
        uint32_t size;
    };

    class PropertyListIterator : public PropertyIteratorImpl {
        PropertyRef _cur;
    public:
        PropertyListIterator(const PropertyList *l)
            : _cur(l)
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
    // be stored, and a PropertyRef, which (if set) refers to
    // suitable unused space in the property list.
    // The _exact flag is set if the property value to be stored
    // is an inline string, which must be stored in a space that
    // exactly matches the length of the string. All other
    // property values can be stored in a space that exceeds the
    // space required.
    class PropertyList::PropertySpace {
        unsigned char _min;
        bool _exact;
        PropertyRef _pos;

    public:
        PropertySpace(int m, bool e = false)
            : _min((unsigned char)m), _exact(e), _pos()
            { }
        operator bool() const { return _pos; }
        const PropertyRef &pos() const { return _pos; }
        int min() const { return _min; }
        bool exact() const { return _exact; }
        bool match(const PropertyRef &p) const;
        void set_pos(const PropertyRef &p) { _pos = p; }
        void set_property(StringID id, const Property &,
                          TransactionImpl *, Allocator &);
    };
};


void PropertyList::init(size_t size)
{
    // Size must be at least 12: 1 byte for the size, 3 bytes for
    // the property info (id and type), and 8 bytes for a link.
    assert(size >= 12 && size <= 255);

    // This is called only for a newly allocated property chunk,
    // so no logging is required.
    _chunk0[0] = uint8_t(size);
    PropertyRef p(this);
    p.set_end(NULL);
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
    return PropertyIterator(new PropertyListIterator(this));
}

void PropertyList::set_property(StringID id, const Property &property)
{
    TransactionImpl *tx = TransactionImpl::get_tx();
    Allocator &allocator = tx->get_db()->allocator();
    PropertyRef pos;
    PropertySpace space(get_space(property));

    if (find_property(id, pos, &space))
        pos.free(tx);

    if (!space) {
        space.set_pos(pos);
        find_space(space, tx, allocator);
    }

    space.set_property(id, property, tx, allocator);
}

void PropertyList::remove_property(StringID id)
{
    PropertyRef p;
    if (find_property(id, p))
        p.free(TransactionImpl::get_tx());
}


// Search the property list for the specified property id.
// If it is found, return a reference to the property in r.
// If the space parameter is non-null, also look for space
// that matches the space requirement.
// If the property is not found, and space is non-null, set
// r to the end of the list, so that the caller can resume
// searching for space where this function left off.
bool PropertyList::find_property(StringID id, PropertyRef &r,
                                 PropertySpace *space) const
{
    PropertyRef p(this);
    while (p.not_done()) {
        switch (p.ptype()) {
            case PropertyRef::p_link:
                p.follow_link();
                continue;
            case PropertyRef::p_unused:
                if (space != NULL && space->match(p)) {
                    // We try for exact fit. If we don't find it, use worst fit.
                    bool exact = p.size() == space->min();
                    if (!*space || exact || p.size() > space->pos().size())
                        space->set_pos(p);
                    // If we got an exact fit, quit looking.
                    if (exact)
                        space = NULL;
                }
                break;
            default:
                if (p.id() == id) {
                    r = p;
                    return true;
                }
                break;
        }
        p.skip();
    }

    if (space != NULL)
        r = p;
    return false;
}


// Search for space suitable to meet the specified space requirement.
// Set the pos member of the space parameter to the space found.
// If no suitable space is found, allocate a new chunk and link to it.
void PropertyList::find_space(PropertySpace &space, TransactionImpl *tx,
                              Allocator &allocator) const
{
    PropertyRef p = space.pos();
    while (p.not_done()) {
        switch (p.ptype()) {
            case PropertyRef::p_link:
                p.follow_link();
                continue;
            case PropertyRef::p_unused:
                if (space.match(p)) {
                    space.set_pos(p);
                    return;
                }
                break;
            default:
                break;
        }
        p.skip();
    }

    // We're at p_end. If there's not space in this chunk,
    // allocate another chunk.
    if (p.check_space(space.min())) {
        space.set_pos(p);
    }
    else {
        // Allocate and initialize a new property chunk
        PropertyList *p_chunk = static_cast<PropertyList *>(allocator.alloc(PropertyList::chunk_size));
        p_chunk->init(PropertyList::chunk_size);

        // If there isn't space for the link in the current chunk,
        // copy some properties from this chunk to the new one.
        // Make_space sets p to the beginning of the space freed up
        // for the link, and sets q to the beginning of free space
        // in the new chunk.
        PropertyRef q(p_chunk);
        if (!p.check_space(sizeof p_chunk))
            p.make_space(q);

        p.set_link(p_chunk, tx);
        space.set_pos(q);
    }
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
    while (p._offset + p.size() + 3 <= chunk_size() - (3 + sizeof (PropertyList *)))
        p.skip();

    *this = p;

    while (p.not_done()) {
        unsigned size = p.get_space();
        q.set_size(size);
        q.set_id(p.id());
        q.set_type(p.ptype());
        memcpy(q.val(), p.val(), size);
        q.skip();
        p.skip();
    }
}

// Determine the minimum number of bytes required to store
// the specified value.
static int get_int_len(long long v)
{
    if (v == 0) return 1;
    if (v < 0) v = -v;
    return bsr(v) / CHAR_BIT + 1;
}


// Determine the number of bytes required to store the referenced value.
int PropertyRef::get_space() const
{
    switch (ptype()) {
        case p_novalue: return 0;
        case p_boolean_false: return 0;
        case p_boolean_true: return 0;
        case p_integer: return get_int_len(int_value());
        case p_string: return size();
        case p_string_ptr: return sizeof (BlobRef);
        case p_float: return sizeof (double);
        case p_time: return sizeof (Time);
        case p_blob: return sizeof (BlobRef);
        default: assert(0);
    }
}


// Determine whether the space referred by p to is suitable.
bool PropertyList::PropertySpace::match(const PropertyRef &p) const
{
    if (_exact)
        return p.size() == _min || p.size() >= _min + 3;
    else
        return p.size() >= _min;
}


// Determine the number of bytes required to store the property value.
PropertyList::PropertySpace PropertyList::get_space(const Property &p)
{
    switch (p.type()) {
        case t_novalue: return 0;
        case t_boolean: return 0;
        case t_integer: return get_int_len(p.int_value());
        case t_string: {
            size_t len = p.string_value().length();
            if (len <= 15) return PropertySpace((unsigned char)len, true);
            return sizeof (PropertyRef::BlobRef);
        }
        case t_float: return sizeof (double);
        case t_time: return sizeof (Time);
        case t_blob: return sizeof (PropertyRef::BlobRef);
        default: assert(0);
    }
}


// Store the property in the space referred to by _pos.
// The caller should have found suitable space.
// The asserts verify that this was done correctly.
void PropertyList::PropertySpace::set_property(StringID id, const Property &p,
                                               TransactionImpl *tx,
                                               Allocator &allocator)
{
    assert(_min == get_space(p)._min);
    assert(_exact == get_space(p)._exact);
    assert(_pos._offset <= _pos.chunk_size() - 3);
    assert((_pos.ptype() == PropertyRef::p_end && _pos.check_space(_min))
           || _pos.size() == _min
           || _pos.size() >= _min + 3
           || (!_exact && _pos.size() >= _min));

    int log_size = _min + 3;
    if (_pos.ptype() == PropertyRef::p_end
            ? _pos._offset + _min + 3 <= _pos.chunk_size() - 3
            : _pos.size() >= _min + 3)
        log_size += 3;

    tx->log(&_pos._chunk[_pos._offset], log_size);

    if (_pos.ptype() == PropertyRef::p_end || _pos.size() >= _min + 3)
        _pos.set_size(_min);

    _pos.set_id(id);
    _pos.set_value(p, allocator);
}


void PropertyRef::free(TransactionImpl *tx)
{
    PropertyRef next(*this, size());
    if (!next.not_done())
        set_end(tx);
    else if (next.ptype() == p_unused) {
        // if the next entry is already free, combine them
        unsigned total_size = size() + next.size() + 3;
        if (total_size <= 15)
            tx->write(&type_size(), uint8_t(total_size << 4));
        else {
            unsigned new_size = total_size >= 18 ? 15 : total_size - 3;
            unsigned new_next_size = total_size - 3 - new_size;
            PropertyRef new_next(*this, new_size);
            tx->log_range(&type_size(), &new_next.type_size());
            type_size() = uint8_t(new_size << 4 | p_unused);
            new_next.type_size() = uint8_t(new_next_size << 4 | p_unused);
        }
    }
    else
        tx->write(&type_size(), uint8_t(size() << 4 | p_unused));
}

void PropertyRef::set_end(TransactionImpl *tx)
{
    if (tx != NULL)
        tx->log_range(&get_id(), &type_size());
    set_id(0);
    type_size() = p_end;
}

// Insert a new property in the property list.
// If the new property is being added at the end,
// mark the following space as the new end,
// otherwise, mark the following space as unused.
void PropertyRef::set_size(int new_size)
{
    if (ptype() == p_end) {
        int unused_size = (chunk_size() - _offset) - (3 + new_size);
        assert(unused_size >= 0);
        if (unused_size >= 3) {
            PropertyRef next(*this, new_size);
            next.type_size() = p_end;
        }
    }
    else {
        int unused_size = size() - new_size;
        assert (unused_size >= 3);
        PropertyRef next(*this, new_size);
        next.type_size() = uint8_t((unused_size - 3) << 4 | p_unused);
    }
    type_size() = uint8_t(new_size << 4);
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
// If there are 3 or more bytes more space than is required,
// put the excess prior to the link, marked as unused.
// If there are less than 3 bytes excess, put the excess
// after the link, where it will be ignored.
void PropertyRef::set_link(PropertyList *p_chunk, TransactionImpl *tx)
{
    unsigned remaining_size = chunk_size() - _offset;
    int unused_size = int(remaining_size) - (3 + int(sizeof p_chunk));
    assert(unused_size >= 0 && unused_size <= 3 + 15);
    tx->log(&_chunk[_offset], remaining_size);
    if (unused_size >= 3) {
        set_id(0);
        type_size() = uint8_t((unused_size - 3) << 4) | p_unused;
        _offset += unused_size;
    }
    set_id(0);
    type_size() = uint8_t(sizeof p_chunk << 4) | p_link;
    *(PropertyList **)val() = p_chunk;
}


// Store the property value in the space referred to.
void PropertyRef::set_value(const Property &p, Allocator &allocator)
{
    assert(_offset <= chunk_size() - 3);
    switch (p.type()) {
        case t_novalue:
            set_type(p_novalue);
            break;
        case t_boolean:
            set_type(p.bool_value() ? p_boolean_true : p_boolean_false);
            break;
        case t_integer: {
            long long v = p.int_value();
            assert(size() >= get_int_len(v));
            set_type(p_integer);
            int len = std::min(size(), (int)sizeof v);
            memcpy(val(), &v, len);
            break;
        }
        case t_string: {
            std::string value = p.string_value();
            size_t len = value.length();
            if (len <= 15) {
                assert(size() == (int)len);
                set_type(p_string);
                memcpy(val(), value.data(), len);
            }
            else {
                set_blob(value.data(), len, allocator);
                set_type(p_string_ptr);
            }
            break;
        }
        case t_float:
            assert(size() >= (int)sizeof (double));
            set_type(p_float);
            *(double *)val() = p.float_value();
            break;
        case t_time:
            assert(size() >= (int)sizeof (Time));
            set_type(p_time);
            *(Time *)val() = p.time_value();
            break;
        case t_blob: {
            Property::blob_t value = p.blob_value();
            set_blob(value.value, value.size, allocator);
            set_type(p_blob);
            break;
        }
        default:
            assert(0);
    }
}

void PropertyRef::set_blob(const void *value, std::size_t size,
                           Allocator &allocator)
{
    assert(this->size() >= (int)sizeof (BlobRef));
    if (size > UINT_MAX) throw Exception(not_implemented);
    void *p = allocator.alloc(size);
    memcpy(p, value, size);
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
        unsigned sz = size();
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
        case p_string: return std::string((const char *)val(), size());
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
        default: assert(0);
    }
}
