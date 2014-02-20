#include <string>
#include "stringid.h"
#include "exception.h"

Jarvis::StringID::StringID(const char *s)
{
    if (s == NULL)
        _id = 0;
    else
        throw e_not_implemented;
}

std::string Jarvis::StringID::name() const
{
    return std::to_string(_id);
}
