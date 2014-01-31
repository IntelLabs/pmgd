#pragma once

#include "stringid.h"

namespace Jarvis {
    class Time { /* TBD */ };
    class PropertyValue
    {
    public:
        PropertyValue(); // property with no value
        PropertyValue(bool);
        PropertyValue(long long);
        PropertyValue(const char *str); // null terminated
        PropertyValue(const char *str, std::size_t len);
        PropertyValue(const std::string str);
        PropertyValue(double);
        PropertyValue(Time);
        PropertyValue(const void *blob, std::size_t size);

        bool operator<(const PropertyValue &);

        bool bool_value() const;
        unsigned long long int_value() const;
        std::string string_value() const;
        double float_value() const;
        Time time_value() const;
        void *blob_value() const;
    };

    class Property {
        StringID _id;
        PropertyValue _value;

    public:
        Property(StringID id, PropertyValue value) : _id(id), _value(value) { }
        StringID id() const { return _id; }
        PropertyValue value() const { return _value; }
    };

    struct PropertyPredicate {
        StringID id;
        enum op_t { eq, ne, gt, ge, lt, le, gele, gelt, gtle, gtlt } op;
        PropertyValue v1, v2;
        PropertyPredicate();
        PropertyPredicate(StringID id); // value is don't care
        PropertyPredicate(StringID id, op_t op,
            const PropertyValue &val);
        PropertyPredicate(StringID id, op_t op,
            const PropertyValue &val1, const PropertyValue &val2);
    };
};
