/**
 * @file   StringTable.cc
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

#include <string.h>       // For memset, memcpy
#include <assert.h>
#include "StringTable.h"
#include "TransactionImpl.h"
#include "exception.h"
#include "arch.h"

using namespace PMGD;

StringTable::StringTable(const uint64_t region_addr, size_t len,
                         unsigned stringid_len, CommonParams &params)
    : MAX_STRINGID_LEN(stringid_len),
      NUM_HASH_BITS(bsr(len/stringid_len)),
      HASH_MASK((1 << NUM_HASH_BITS) - 1),
      _pm(reinterpret_cast<char *>(region_addr))
{
    size_t num_entries = len / stringid_len;
    (void)num_entries; // Silence unused variable warning
    assert((num_entries & (num_entries - 1)) == 0 );
    assert(num_entries <= (1 << 16));

    if (params.create) {
        // StringTable has to surely be zeroed out.
        // TODO Remove this in case PMFS or any other memory management
        // layer ensures that.
        memset(_pm, 0, len);
        // Cannot use write_nolog() since this is graph init time
        TransactionImpl::flush_range(_pm, len, params.msync_needed,
                                     *params.pending_commits);
    }
}

// Implementation of a 16-bit Fowler-Noll-Vo FNV-1a hash function.
// (see http://en.wikipedia.org/wiki/Fowler_Noll_Vo_hash and
// http://www.isthe.com/chongo/tech/comp/fnv/#xor-fold)
inline uint16_t StringTable::hash_fnv_1a(const char *name, unsigned &len)
{
    static const unsigned FNV_prime = (1<<24) + (1<<8) + 0x93;
    static const unsigned OFFSET_BASIS = 2166136261;
    uint32_t hash = OFFSET_BASIS;
    unsigned index = 0;

    while(name[index] != 0) {
        hash ^= (unsigned char)name[index];
        hash *= FNV_prime;
        ++index;
    }
    len = index;

    // Fold into a 16-bit hash value first, then use as many bits
    // as permitted by table size. The source recommends implementing
    // xor-fold by shifting the 32-bit hash right based on number of bits
    // needed. We measured fewer collisions by shifting by the constant 16.
    return (uint16_t)(((hash >> 16) ^ hash) & HASH_MASK);
}

bool StringTable::get(const char *name, uint16_t &id, bool add)
{
    bool r;
    unsigned length;

    uint16_t hash = hash_fnv_1a(name, length);

    if (length > MAX_STRINGID_LEN)
        throw PMGDException(InvalidID);

    while (true) {
        uint64_t offset = hash * MAX_STRINGID_LEN;
        char *dest = _pm + offset;
        // Check if the slot is unoccupied.
        if (*dest == 0) {
            // Found an empty slot.
            if (add) {
                // Ok to acquire transaction object here again (after StringID)
                // because this is not a frequented branch.
                TransactionImpl *tx = TransactionImpl::get_tx();
                tx->check_read_write();
                tx->write_nolog(dest, (void *)name, length);
            }
            else
                hash = 0;
            r = false;
            break;
        }
        // Some string there already. Check if it matches with name.
        // Using 32-bit int instead of 64 since some strings could be
        // really small.
        const uint32_t *s1 = reinterpret_cast<const uint32_t *>(name);
        uint32_t *s2 = reinterpret_cast<uint32_t *>(dest);
        int l = length % 4;
        int num_ints = length / 4;

        bool match = true; // Assume we will succeed in this comparison
        for (int i = 0; i < num_ints; ++i) {
            if (s1[i] != s2[i]) {
                match = false;
                break;
            }
        }
        if (l > 0) {
            char *s = (char *)(s1 + num_ints);
            char *d = (char *)(s2 + num_ints);
            for (int i = 0; i < l; ++i) {
                if (s[i] != d[i]) {
                    match = false;
                    break;
                }
            }
        }
        if (match) {
            r = true;
            break;
        }
        hash = uint16_t((hash + 1) & HASH_MASK);
    }

    id = hash;
    return r;
}

std::string StringTable::get(uint16_t id) const
{
    uint64_t offset = id * MAX_STRINGID_LEN;
    char *str = _pm + offset;
    unsigned length = 0;
    while (length < MAX_STRINGID_LEN && *str != '\0') {
        ++length;
        ++str;
    }
    return std::string(_pm + offset, length);
}
