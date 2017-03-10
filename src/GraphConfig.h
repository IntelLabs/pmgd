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

#pragma once

#include <string>
#include <vector>

#include "graph.h"
#include "os.h"
#include "RangeSet.h"

namespace PMGD {
    // We pass the same set of variables in a lot of constructors.
    // So let's collect them together.
    struct CommonParams {
        bool create;
        bool read_only;
        bool msync_needed;    // false only when NoMsync used for msync cases.
        bool always_msync;    // true only when AlwaysMsync used for msync cases.

        // Make this a pointer so we have the option of setting
        // it to the transactional data structure when a transaction is
        // active. At graph create time, we can allocate a new one.
        RangeSet *pending_commits;  // Needed to track graph time page writes for msync

        // Most common use case of this structure is to pass
        // create and msync parameters. So make the common constructor.
        CommonParams(bool c, bool m) :
            CommonParams(c, false, m, false, m ? new RangeSet() : NULL)
          {}

        CommonParams(bool c, bool r, bool m, bool a, RangeSet *pc)
        {
            create = c;  read_only = r;
            msync_needed = m; always_msync = a;
            pending_commits = pc;
        }
    };

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
        unsigned num_allocators;

        // The parameters below until locale_name are DRAM-based parameters
        // that can be modified each time the graph is created/opened.
        // The variables above and locale_name onwards are PM-based parameters
        // which are fixed once the graph
        // is created.
        // Size of memory area allocated for locks.
        size_t node_striped_lock_size;    // bytes
        size_t edge_striped_lock_size;    // bytes
        size_t index_striped_lock_size;   // bytes

        // Number of bytes from a given address that are
        // considered locked with one striped lock unit.
        unsigned node_stripe_width;
        unsigned edge_stripe_width;
        unsigned index_stripe_width;

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
