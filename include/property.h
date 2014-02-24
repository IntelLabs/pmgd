#pragma once

#include <stddef.h>
#include <assert.h>
#include <string>
#include "exception.h"
#include "stringid.h"

namespace Jarvis {
    enum PropertyType { t_novalue = 1, t_boolean, t_integer, t_string,
                        t_float, t_time, t_blob };

    struct Time {
        bool operator<(const Time &) const { throw e_not_implemented; }
    };

    class PropertyValue {
    public:
        struct blob_t {
            const void *value;
            std::size_t size;
            blob_t(const void *v, std::size_t s) : value(v), size(s) { }
        };

    private:
        PropertyType _type;
        union {
            bool v_boolean;
            long long v_integer;
            std::string v_string;
            double v_float;
            Time v_time;
            blob_t v_blob;
        };

        void check(int t) const { if (_type != t) throw e_property_type; }

    public:
        PropertyValue() : _type(t_novalue) { }
        PropertyValue(const PropertyValue &);
        PropertyValue(bool v) : _type(t_boolean), v_boolean(v) { }
        PropertyValue(int v) : _type(t_integer), v_integer(v) { }
        PropertyValue(long long v) : _type(t_integer), v_integer(v) { }
        PropertyValue(const char *s) : _type(t_string), v_string(s) { }
        PropertyValue(const char *s, std::size_t len)
            : _type(t_string), v_string(s, len) { }
        PropertyValue(const std::string str)
            : _type(t_string), v_string(str) { }
        PropertyValue(double v) : _type(t_float), v_float(v) { }
        PropertyValue(Time v) : _type(t_time), v_time(v) { }
        PropertyValue(const blob_t &blob)
            : _type(t_blob), v_blob(blob) { }
        PropertyValue(const void *blob, std::size_t size)
            : _type(t_blob), v_blob(blob, size) { }

        ~PropertyValue();

        void operator=(const PropertyValue &);

        bool operator<(const PropertyValue &) const;

        PropertyType type() const { return _type; } 
        bool bool_value() const { check(t_boolean); return v_boolean; }
        long long int_value() const { check(t_integer); return v_integer; }
        std::string string_value() const { check(t_string); return v_string; }
        double float_value() const { check(t_float); return v_float; }
        Time time_value() const { check(t_time); return v_time; }
        blob_t blob_value() const { check(t_blob); return v_blob; }
    };

    class Property {
        StringID _id;
        PropertyValue _value;

    public:
        Property() : _id(0) { }
        Property(StringID id, PropertyValue value) : _id(id), _value(value) { }
        StringID id() const { return _id; }
        const PropertyValue &value() const { return _value; }
    };

    struct PropertyPredicate {
        StringID id;
        enum op_t { dont_care,
                    eq, ne, gt, ge, lt, le,
                    gele, gelt, gtle, gtlt } op;
        PropertyValue v1, v2;
        PropertyPredicate() : id(0) { }
        PropertyPredicate(StringID i) : id(i), op(dont_care) { }
        PropertyPredicate(StringID i, op_t o, const PropertyValue &v)
            : id(i), op(o), v1(v) { assert(o > dont_care && o <= le); }
        PropertyPredicate(StringID i, op_t o,
                const PropertyValue &val1, const PropertyValue &val2)
            : id(i), op(o), v1(val1), v2(val2)
            { assert(o >= gele); }
    };

    inline bool operator==(const PropertyValue &a, const PropertyValue &b)
        { return !(a < b && b < a); }
    inline bool operator!=(const PropertyValue &a, const PropertyValue &b)
        { return !(a == b); }
    inline bool operator>(const PropertyValue &a, const PropertyValue &b)
        { return (b < a); }
    inline bool operator<=(const PropertyValue &a, const PropertyValue &b)
        { return !(a > b); }
    inline bool operator>=(const PropertyValue &a, const PropertyValue &b)
        { return !(a < b); }
};
