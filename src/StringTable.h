/**
 * @file   StringTable.h
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
#include <stdint.h>
#include "GraphConfig.h"

namespace PMGD {
    class StringTable {
        // Non-PM counters
        const unsigned MAX_STRINGID_LEN;
        // Count of bits actually used for StringID (<= 16)
        const unsigned NUM_HASH_BITS;
        const unsigned HASH_MASK;

        // Array of 16B char arrays indexed by hash
        // Sits in PM
        char *_pm;

        // Computes FNV hash as well as computes the string's length
        // as an optimization.
        uint16_t hash_fnv_1a(const char *name, unsigned &len);

    public:
        StringTable(const StringTable &) = delete;
        void operator=(const StringTable &) = delete;
        StringTable(const uint64_t region_addr, size_t len,
                    unsigned stringid_len, CommonParams &params);
        bool get(const char *name, uint16_t &id, bool add);
        std::string get(uint16_t id) const;
    };
};
