#include <stddef.h>
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
    class PropertyListIterator : public PropertyIteratorImpl {
        PropertyRef _cur;
    public:
        PropertyListIterator(const PropertyList *l) : _cur(l) { }
        operator bool() const { return _cur.not_done(); }
        const PropertyRef &operator*() const { return _cur; }
        const PropertyRef *operator->() const { return &_cur; }
        PropertyRef &operator*() { return _cur; }
        PropertyRef *operator->() { return &_cur; }
        bool next() { return _cur.next(); }
    };

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
        bool set_property(StringID id, const Property &, Allocator &);
    };
};


void PropertyList::init(size_t size)
{
    // Size must be at least 12: 1 byte for the size, 3 bytes for
    // the property info (id and type), and 8 bytes for a link.
    assert(size >= 12 && size <= 255);
    _chunk0[0] = uint8_t(size);
    PropertyRef p(this);
    p.set_end();
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

    if (find_property(id, pos, &space)) {
        space.set_pos(pos);
        if (space.set_property(id, property, allocator))
            return;
        pos.free();
    }

    if (!space) {
        space.set_pos(pos);
        find_space(space, allocator);
    }

    bool r = space.set_property(id, property, allocator);
    assert(r);
}

void PropertyList::remove_property(StringID id)
{
    PropertyRef p;
    if (find_property(id, p))
        p.free();
}

bool PropertyList::find_property(StringID id, PropertyRef &r,
                                 PropertySpace *space) const
{
    PropertyRef p(this);
    while (1) {
        switch (p.ptype()) {
            case PropertyRef::p_end:
                goto break2;
            case PropertyRef::p_link:
                p.follow_link();
                continue;
            case PropertyRef::p_unused:
                if (space != NULL && space->match(p)) {
                    // We try for exact fit. If we don't find it, use worst fit.
                    bool exact = p.size() == space->min();
                    if (!space || exact || p.size() > space->pos().size())
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
        if (!p.next())
            break;
    }
break2:
    if (space != NULL)
        r = p;
    return false;
}

void PropertyList::find_space(PropertySpace &space, Allocator &allocator) const
{
    PropertyRef p = space.pos();
    if (p.ptype() != PropertyRef::p_end) {
        do {
            if (p.ptype() == PropertyRef::p_unused && space.match(p)) {
                space.set_pos(p);
                return;
            }
        } while (p.next());
    }

    if (p.check_space(space.min())) {
        PropertyRef next(p, space.min());
        next.set_end();
        space.set_pos(p);
        return;
    }

    // Allocate a new property chunk
    throw Exception(not_implemented);
}

bool PropertyList::PropertySpace::match(const PropertyRef &p) const
{
    return p.size() == _min || (!_exact && p.size() >= _min + 3);
}

static int get_int_len(long long v)
{
    if (v == 0) return 1;
    if (v < 0) v = -v;
    return bsr(v) / 8 + 1;
}

PropertyList::PropertySpace PropertyList::get_space(const Property &p)
{
    switch (p.type()) {
        case t_novalue: return 0;
        case t_boolean: return 0;
        case t_integer: return get_int_len(p.int_value());
        case t_string: {
            size_t len = p.string_value().length();
            if (len <= 15) return PropertySpace((unsigned char)len, true);
            return sizeof (void *);
        }
        case t_float: return sizeof (double);
        case t_time: return sizeof (Time);
        case t_blob: return sizeof (void *);
        default: assert(0);
    }
}

bool PropertyList::PropertySpace::set_property(StringID id, const Property &p,
                                               Allocator &allocator)
{
    assert(_min == get_space(p)._min);
    assert(_exact == get_space(p)._exact);
    assert((_pos.ptype() == PropertyRef::p_end && _pos.check_space(_min))
           || _pos.size() == _min
           || _pos.size() >= _min + 3
           || (!_exact && _pos.size() >= _min));
    if (_pos.ptype() == PropertyRef::p_end || _pos.size() >= _min + 3) {
        _pos.set_size(_min);
        _pos.set_id(id);
        _pos.set_value(p, allocator);
        return true;
    }
    else if (_pos.size() == _min || !_exact) {
        _pos.set_id(id);
        _pos.set_value(p, allocator);
        return true;
    }
    else
        return false;
}

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
            size_t len = p.string_value().length();
            if (len <= 15) {
                assert(size() == (int)len);
                set_type(p_string);
                memcpy(val(), p.string_value().data(), len);
            }
            else {
                assert(size() >= (int)sizeof (void *));
                set_type(p_string_ptr);
                throw Exception(not_implemented);
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
            assert(size() >= (int)sizeof (void *));
            set_type(p_blob);
            throw Exception(not_implemented);
            break;
        }
        default:
            assert(0);
    }
}

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
        long long v = *(long long *)val();
        unsigned sz = size();
        if (sz < 8) {
            int shift =  8 * (8 - sz);
            v = v << shift >> shift;
        }
        return v;
    }
    throw Exception(property_type);
}

std::string PropertyRef::string_value() const
{
    switch (ptype()) {
        case p_string: return std::string((const char *)val(), size());
        case p_string_ptr: throw Exception(not_implemented);
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
        throw Exception(not_implemented);
    }
    throw Exception(property_type);
}

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
