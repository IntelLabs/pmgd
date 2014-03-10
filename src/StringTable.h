#pragma once

#include <string>
#include <string.h>       // For strncpy, memset
#include <stdint.h>
#include "exception.h"
#include "arch.h"

namespace Jarvis {
    class StringTable {
        static const unsigned POWER2_16 = 65536;
        // Based on algorithm from:
        // http://www.isthe.com/chongo/tech/comp/fnv/#FNV-1a
        static const unsigned POWER2_24 = 16777216;
        static const unsigned POWER2_8 = 256;
        static const unsigned FNV_prime = (POWER2_24 + POWER2_8 + 0x93);
        static const unsigned OFFSET_BASIS = 2166136261;

        // Non-pm counters
        // Used as constants but determined in constructor
        int MAX_STRINGID_LEN;
        // Specific size specifier for hash
        uint16_t HASH_BITS; 
        uint16_t LAST_HASH_BITS;

        // Array of 16B char arrays indexed by hash
        // Sits in PM
        char *_pm;

        // TODO Perhaps one of a common index functions?
        uint16_t hash_fnv_1a(const char *name, int *len);

    public:

        // ** Needs a 0ed out buffer
        StringTable(const uint64_t region_addr, size_t len, int stringid_len, bool create)
            : _pm(reinterpret_cast<char *>(region_addr))
        {
            size_t num_entries = len / stringid_len;
            if ( (num_entries & (num_entries - 1)) != 0 || num_entries > POWER2_16)
                throw Exception(bad_alloc);

            if (create)
                memset(_pm, 0, len);
            
            MAX_STRINGID_LEN = stringid_len;
            HASH_BITS = (uint16_t)bsr(num_entries);
            LAST_HASH_BITS = (uint16_t)((1 << HASH_BITS) - 1);
        }
        uint16_t get(const char *name);
        std::string get(uint16_t id) const;
    };
};
