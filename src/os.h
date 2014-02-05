#pragma once

#include <stdint.h>

namespace Jarvis {
    namespace os {
        class MapHandle {
            int _fd;

        public:
            MapHandle(const char *dir, const char *name,
                      uint64_t map_addr, uint64_t map_len,
                      bool &create, bool truncate);

            ~MapHandle();
        };
    };
};
