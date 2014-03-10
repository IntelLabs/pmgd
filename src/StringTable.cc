#include "StringTable.h"
#include "TransactionImpl.h"

using namespace Jarvis;

// Based on algorithm from:
// http://www.isthe.com/chongo/tech/comp/fnv/#FNV-1a
uint16_t StringTable::hash_fnv_1a(const char *name, int *len)
{
    uint32_t hash = OFFSET_BASIS;
    uint16_t return_hash;
    int idx = 0;

    while(name[idx] != 0) {
        hash = hash ^ name[idx];
        hash = hash * FNV_prime;
        ++idx;
    }

    *len = idx;
    return_hash = (uint16_t)((hash >> (32 - HASH_BITS)) ^ (hash & LAST_HASH_BITS));

    return return_hash;
}

uint16_t StringTable::get(const char *name)
{
    bool collision = true;
    int length;
    uint16_t factor = 1;

    uint16_t hash = hash_fnv_1a(name, &length);

    // If the hash portion was not empty, check if some
    // other string occupying area and resolve collisions,
    // if any
    while (collision) {
        uint64_t offset = hash * MAX_STRINGID_LEN;
        char *dest = _pm + offset;
        if (*dest == 0) {  // 1st B 0 ==> no string there
            strncpy(dest, name, length);
            TransactionImpl::flush_range(dest, length);
            break; // hashing succeeded, breal from collision loop
        }
        // Some string there already
        // Using 32bit instead of 64 since some strings could be
        // really small
        const uint32_t *s1 = reinterpret_cast<const uint32_t *>(name);
        uint32_t *s2 = reinterpret_cast<uint32_t *>(dest);
        int l = length % 4;
        int num_ints = length / 4;

        collision = false; // Assume we will succeed in this comparison
        for (int i = 0; i < num_ints; ++i) {
            if ( s1[i] != s2[i] ) {
                collision = true;
                break;
            }
        }
        if (l > 0) {
            char *s = (char *)(s1 + num_ints);
            char *d = (char *)(s2 + num_ints);
            for (int i = 0; i < l; ++i) {
                if (s[i] != d[i])
                    collision = true;
            }
        }
        if (collision) { // collision in the two little letters
            // Quadratic resolution
            hash = (uint16_t)((hash + (factor * factor)) & LAST_HASH_BITS);
            ++factor;
        }
    }  // Make sure string is hashed
    return hash;
}
        
std::string StringTable::get(uint16_t id) const
{
    uint64_t offset = id * MAX_STRINGID_LEN;
    return std::string((const char *)(_pm + offset), MAX_STRINGID_LEN);
}
