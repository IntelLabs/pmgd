/**
 * @file   GraphConfig.h
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

#include <string>
#include <vector>

#include "graph.h"
#include "os.h"

namespace PMGD {
    struct RegionInfo {
        static const int REGION_NAME_LEN = 32;
        char name[REGION_NAME_LEN];
        uint64_t addr;
        size_t len;
    };

    struct AllocatorInfo {
        unsigned object_size;
        uint64_t offset;
        uint64_t pool_size;
    };

    struct GraphConfig {
        static const size_t BASE_ADDRESS = SIZE_1TB;
        static const size_t INFO_SIZE = SIZE_4KB;

        unsigned node_size;
        unsigned edge_size;
        unsigned max_stringid_length;
        std::string locale_name;

        RegionInfo transaction_info;
        RegionInfo journal_info;
        RegionInfo indexmanager_info;
        RegionInfo stringtable_info;
        RegionInfo node_info;
        RegionInfo edge_info;
        RegionInfo allocator_info;

        GraphConfig(const Graph::Config *user_config);
        void init_region_info(RegionInfo &info, const char *name,
                              uint64_t &addr, size_t size);
    };
};
