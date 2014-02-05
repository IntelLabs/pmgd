#pragma once

#include <stdint.h>

namespace Jarvis {
    namespace os {
        class MapRegion {
            int _fd;

        public:
            MapRegion(const char *db_name, const char *region_name,
                      uint64_t map_addr, uint64_t map_len,
                      bool &create, bool truncate);

            ~MapRegion();
        };
    };
};
