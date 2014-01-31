#pragma once

namespace Jarvis {
    class StringID {
    public:
        StringID(const char *);
        bool operator<(const StringID &);
    };
};
