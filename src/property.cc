#include "property.h"
#include "iterator.h"

Jarvis::PropertyValue::PropertyValue(const PropertyValue &a)
    : _type(a._type)
{
    switch (a._type) {
        case t_novalue: break;
        case t_boolean: v_boolean = a.v_boolean; break;
        case t_integer: v_integer = a.v_integer; break;
        case t_string: v_string = a.v_string; break;
        case t_float: v_float = a.v_float; break;
        case t_time: v_time = a.v_time; break;
        case t_blob: v_blob = a.v_blob; break;
        default: throw e_property_type;
    }
}

Jarvis::PropertyValue::~PropertyValue()
{
    if (_type == t_string) {
        v_string.std::string::~string();
    }
}

bool Jarvis::PropertyValue::operator<(const PropertyValue &a)
{
    check(a._type);

    switch (_type) {
        case t_novalue: return false; // no ordering
        case t_boolean: return v_boolean < a.v_boolean;
        case t_integer: return v_integer < a.v_integer;
        case t_string: return v_string < a.v_string; // collation order!
        case t_float: return v_float < a.v_float;
        case t_time: return v_time < a.v_time;
        case t_blob: return false; // no ordering
    }

    throw e_property_type;
}

Jarvis::PropertyValueRef::operator PropertyValue() const
{
    switch (type()) {
        case t_novalue: return PropertyValue();
        case t_boolean: return PropertyValue(bool_value());
        case t_integer: return PropertyValue(int_value());
        case t_string: return PropertyValue(string_value());
        case t_float: return PropertyValue(float_value());
        case t_time: return PropertyValue(time_value());
        case t_blob: return PropertyValue(blob_value());
    }

    throw e_property_type;
}
