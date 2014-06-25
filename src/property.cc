#include "property.h"
#include "iterator.h"

Jarvis::Property::Property(const Property &a)
    : _type(a._type)
{
    switch (a._type) {
        case t_novalue: break;
        case t_boolean: v_boolean = a.v_boolean; break;
        case t_integer: v_integer = a.v_integer; break;
        case t_string: new(&v_string) std::string(a.v_string); break;
        case t_float: v_float = a.v_float; break;
        case t_time: v_time = a.v_time; break;
        case t_blob: v_blob = a.v_blob; break;
        default: assert(0);
    }
}

Jarvis::Property::~Property()
{
    if (_type == t_string) {
        v_string.std::string::~string();
    }
}

void Jarvis::Property::operator=(const Property &a)
{
    if (_type == t_string)
        v_string.std::string::~string();
    _type = a._type;
    switch (a._type) {
        case t_novalue: break;
        case t_boolean: v_boolean = a.v_boolean; break;
        case t_integer: v_integer = a.v_integer; break;
        case t_string: new(&v_string) std::string(a.v_string); break;
        case t_float: v_float = a.v_float; break;
        case t_time: v_time = a.v_time; break;
        case t_blob: v_blob = a.v_blob; break;
        default: assert(0);
    }
}

bool Jarvis::Property::operator<(const Property &a) const
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
        default: assert(0); return false;
    }
}
