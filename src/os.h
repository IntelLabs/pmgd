/**
 * @file   os.h
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include "RangeSet.h"

namespace PMGD {
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
            MapRegion(const MapRegion &) = delete;
            void operator=(const MapRegion &) = delete;

            MapRegion(const char *db_name, const char *region_name,
                      uint64_t map_addr, uint64_t map_len,
                      bool &create, bool truncate, bool read_only);

            ~MapRegion();
        };

        class SigHandler {
            static void sigbus_handler(int);
        public:
            SigHandler(const SigHandler &) = delete;
            void operator=(const SigHandler &) = delete;
            SigHandler();
        };

        size_t get_default_region_size();
        size_t get_alignment(size_t size);

        void flush(void *addr, RangeSet &pending_commits);
        void commit(RangeSet &pending_commits);
    };
};
