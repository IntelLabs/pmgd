#pragma once

#include <string>

namespace Jarvis {
    class StringID {
        uint16_t _id;
    public:
        StringID(const char *);
        bool operator<(const StringID &a) const { return _id < a._id; }
        bool operator==(const StringID &a) const { return _id == a._id; }
        std::string name();
    };
};
