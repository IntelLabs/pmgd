#pragma once

#include <string>
#include <stdint.h>

namespace Jarvis {
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
                    unsigned stringid_len, bool create);
        bool get(const char *name, uint16_t &id, bool add);
        std::string get(uint16_t id) const;
    };
};
