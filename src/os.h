#pragma once

#include <stdint.h>

namespace Jarvis {
    // some useful constants
    static const size_t SIZE_1TB = 0x10000000000;
    static const size_t SIZE_1GB = 0x40000000;
    static const size_t SIZE_2MB = 0x200000;
    static const size_t SIZE_4KB = 0x1000;

    namespace os {
        class MapRegion {
            class OSMapRegion;
            OSMapRegion *_s;

        public:
            MapRegion(const char *db_name, const char *region_name,
                      uint64_t map_addr, uint64_t map_len,
                      bool &create, bool truncate, bool read_only);

            ~MapRegion();
        };

        class SigHandler {
            static void sigbus_handler(int);
        public:
            SigHandler();
        };

        size_t get_default_region_size();
        size_t get_alignment(size_t size);
    };
};
