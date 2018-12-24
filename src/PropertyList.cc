/**
 * @file   PropertyList.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

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

using namespace PMGD;

namespace PMGD {
#pragma pack(push, 1)
    struct PropertyRef::BlobRef {
        void *value;
        uint32_t size;
    };
#pragma pack(pop)

    class PropertyListIteratorImpl : public PropertyIteratorImplIntf {
        PropertyRef _cur;
        bool _vacant_flag = false;
        TransactionImpl *_tx;

    public:
        PropertyListIteratorImpl(const PropertyList *list)
            : _cur(list),
              _tx(TransactionImpl::get_tx())
        {
            if (_cur.skip_to_next()) {
                _tx->iterator_callbacks().register_property_iterator(this,
                        [this](const PropertyRef &p) { notify(p); });
            }
            else {
                _cur._chunk = NULL;
            }
        }

        ~PropertyListIteratorImpl()
        {
            _tx->iterator_callbacks().unregister_property_iterator(this);
        }

        operator bool() const { return _vacant_flag || _cur._chunk != NULL; }

        PropertyRef *ref()
        {
            if (_vacant_flag)
                throw PMGDException(VacantIterator);
            return &_cur;
        }

        bool next()
        {
            // If _vacant_flag is set, the iterator has already advanced
            // to the next property, so just clear _vacant_flag.
            if (_vacant_flag) {
                _vacant_flag = false;
                return operator bool();
            }

            if (!_cur.next()) {
                _cur._chunk = NULL;
                return false;
            }

            return true;
        }

        void notify(const PropertyRef &p)
        {
            // If the property referred to by the iterator is being
            // removed, move to the next property, and set _vacant_flag
            // to indicate that the current property no longer exists.
            // If p._offset is 0, it means that the entire property list
            // is being removed.
            if (p._chunk == _cur._chunk) {
                if (p._offset == 0)
                    _cur._chunk = NULL;
                else if (p._offset == _cur._offset) {
                    if (!_cur.next())
                        _cur._chunk = NULL;
                }
                _vacant_flag = true;
            }
        }
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
        throw PMGDException(PropertyNotFound, id.name());

    return p.get_value();
}

PropertyIterator PropertyList::get_properties() const
{
    return PropertyIterator(new PropertyListIteratorImpl(this));
}


// PropertyList:set_property handles updating the index as well
// as updating the property list.
// This is because we need to check whether there is an existing
// property with the given id and remove it from the index before
// setting the new property value and adding it to the index.
// The index manager also checks that the index, if any, is the
// same type as the property value being set.
//
// index_type is of type int because otherwise there would be a circular
// dependency between graph.h and iterator.h.
// graph.h needs Iterator, and iterator.h would need Graph::IndexType.
// Breaking PropertyList.h out into a separate include file wouldn't help;
// it would just make the circular dependency more complicated.
void PropertyList::set_property(StringID id, const Property &new_value,
            /*Graph::IndexType*/ int index_type, StringID tag, void *obj)
{
    TransactionImpl *tx = TransactionImpl::get_tx();
    GraphImpl *db = tx->get_db();
    Allocator &allocator = db->allocator();
    PropertyRef pos;
    PropertySpace space(get_space(new_value) + 3);

    if (find_property(id, pos, &space)) {
        db->index_manager().update(db, Graph::IndexType(index_type), tag, obj,
                                   id, &pos, &new_value);
        pos.free(tx);
    }
    else {
        db->index_manager().update(db, Graph::IndexType(index_type), tag, obj,
                                   id, NULL, &new_value);
    }

    // Mantis #802
    // If any of the following steps throws BadAlloc, these changes
    // have already been made to the graph:
    // 1. The old property value has been removed from the property list.
    // 2. The old property value has been removed from the index.
    // 3. The new property value has been added to the index, if there is
    //    one. This can leave a reference in the index to a node or edge
    //    that doesn't have the indexed property.
    // If the exception causes the transaction to be aborted, these changes
    // will be reverted, but if the application catches the exception and
    // commits the transaction, these incorrect changes will become persistent.
    // This might be a good place to use a dependent nested transaction,
    // once that is implemented.

    if (space.size() == 0) {
        if (!find_space(space, pos)) {
            add_chunk(pos, tx, allocator);
            space.set_pos(pos, pos.free_space());
            space.set_new();
        }
    }

    space.set_property(id, new_value, tx, allocator);
}


void PropertyList::remove_property(StringID id,
            /*Graph::IndexType*/ int index_type, StringID tag, void *obj)
{
    PropertyRef p;
    if (find_property(id, p)) {
        TransactionImpl *tx = TransactionImpl::get_tx();
        GraphImpl *db = tx->get_db();
        db->index_manager().update(db, Graph::IndexType(index_type), tag, obj,
                                   id, &p, NULL);
        p.free(tx);
    }
}

void PropertyList::remove_all_properties(
            /*Graph::IndexType*/ int index_type, StringID tag, void *obj)
{
    // Free all blobs and strings, free all chunks beyond chunk0,
    // and set the first byte of chunk0 to p_end, indicating that
    // there are no properties in the list.
    TransactionImpl *tx = TransactionImpl::get_tx();
    GraphImpl *db = tx->get_db();
    Allocator &allocator = db->allocator();
    PropertyRef p(this);
    bool first = true;
    while (p.not_done()) {
        switch (p.ptype()) {
            case PropertyRef::p_link: {
                uint8_t *chunk = p._chunk;
                p.follow_link();
                tx->iterator_callbacks().property_iterator_notify(PropertyRef(chunk, 0));
                if (!first)
                    allocator.free(chunk, PropertyList::chunk_size);
                first = false;
                continue;
            }
            case PropertyRef::p_unused:
                break;
            case PropertyRef::p_string_ptr:
                db->index_manager().update(db, Graph::IndexType(index_type),
                                           tag, obj, p.get_id(), &p, NULL);
                /* fall through */
            case PropertyRef::p_blob: { // Note: indexes not supported for blobs
                PropertyRef::BlobRef *v = (PropertyRef::BlobRef *)p.val();
                allocator.free(v->value, v->size);
                break;
            }
            default:
                db->index_manager().update(db, Graph::IndexType(index_type),
                                           tag, obj, p.get_id(), &p, NULL);
                break;
        }
        p.skip();
    }
    tx->iterator_callbacks().property_iterator_notify(PropertyRef(p._chunk, 0));
    if (!first)
        allocator.free(p._chunk, PropertyList::chunk_size);
    tx->write<uint8_t>(&_chunk0[1], PropertyRef::p_end);
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
        case PropertyType::NoValue: return 0;
        case PropertyType::Boolean: return 0;
        case PropertyType::Integer: return get_int_len(p.int_value());
        case PropertyType::String: {
            size_t len = p.string_value().length();
            if (len <= 13) return len;
            return sizeof (PropertyRef::BlobRef);
        }
        case PropertyType::Float: return sizeof (double);
        case PropertyType::Time: return sizeof (Time);
        case PropertyType::Blob: return sizeof (PropertyRef::BlobRef);
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
        tx->flush_range(_pos._chunk, _pos._offset + log_size);
    }
}


void PropertyRef::free(TransactionImpl *tx)
{
    tx->iterator_callbacks().property_iterator_notify(*this);

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
        case PropertyType::NoValue:
            type = p_novalue;
            break;
        case PropertyType::Boolean:
            type = p.bool_value() ? p_boolean_true : p_boolean_false;
            break;
        case PropertyType::Integer: {
            long long v = p.int_value();
            assert(size == get_int_len(v) + 3);
            memcpy(val(), &v, size - 3);
            type = p_integer;
            break;
        }
        case PropertyType::String: {
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
        case PropertyType::Float:
            *(double *)val() = p.float_value();
            type = p_float;
            break;
        case PropertyType::Time:
            *(Time *)val() = p.time_value();
            type = p_time;
            break;
        case PropertyType::Blob: {
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
    if (size > UINT_MAX) throw PMGDException(NotImplemented);
    void *p = allocator.alloc(size);
    memcpy(p, value, size);
    TransactionImpl *tx = TransactionImpl::get_tx();
    tx->flush_range(p, size);
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
    throw PMGDException(PropertyTypeMismatch);
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
    throw PMGDException(PropertyTypeMismatch);
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
    throw PMGDException(PropertyTypeMismatch);
}

double PropertyRef::float_value() const
{
    if (ptype() == p_float)
        return *(double *)val();
    throw PMGDException(PropertyTypeMismatch);
}

Time PropertyRef::time_value() const
{
    if (ptype() == p_time)
        return *(Time *)val();
    throw PMGDException(PropertyTypeMismatch);
}

Property::blob_t PropertyRef::blob_value() const
{
    if (ptype() == p_blob) {
        BlobRef *v = (BlobRef *)val();
        return Property::blob_t(v->value, v->size);
    }
    throw PMGDException(PropertyTypeMismatch);
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
