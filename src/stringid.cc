#include "stringid.h"
#include "exception.h"

Jarvis::StringID::StringID(const char *s)
{
    if (s == NULL)
        _id = 0;
    else
        throw e_not_implemented;
}

std::string Jarvis::StringID::name()
{
    if (_id == 0)
        return "0";
    else
        throw e_not_implemented;
}
