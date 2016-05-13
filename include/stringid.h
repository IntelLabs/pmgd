#pragma once

#include <string>
#include <stdint.h>

namespace Jarvis {
    class StringID {
        uint16_t _id;
        static bool get(const char *, StringID &stringid, bool add);
    public:
        static bool lookup(const char *name, StringID &stringid)
            { return get(name, stringid, false); }
        StringID(const char *name = 0) { (void)get(name, *this, true); }
        StringID(int id) : _id(id) { }
        bool operator<(const StringID &a) const { return _id < a._id; }
        bool operator==(const StringID &a) const { return _id == a._id; }
        std::string name() const;
        // TODO For debugging only
        uint16_t id() const { return _id; }
    };
};
