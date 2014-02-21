#pragma once

#include <stddef.h>
#include <stdint.h>
#include "os.h"

namespace Jarvis {

    /**
     * Fixed-size allocator parameters
     *
     * Each allocator instance operates on a named region of
     * persistent memory.
     */
    struct AllocatorInfo {
        static const int REGION_NAME_LEN = 32;
        char name[REGION_NAME_LEN];     ///< Region name
        uint64_t addr;                  ///< Virtual address of region
        size_t len;                     ///< Length in byte
        uint32_t size;                  ///< Object size in bytes, size <<< len
        bool zero;                      ///< Zero the region before use
    };

    /**
     * Fixed-size allocator
     *
     * This class defines an allocator of fixed-size objects stored in
     * a region of persistent memory backed by a file.  Users of this
     * class can instantiate as many allocators as they wish, each
     * instance with the same or different object sizes.  Instances of
     * this class live in volatile memory with a pointer to the
     * beginning of the region in persistent memory.
     */
    class FixedAllocator
    {
    private:
        static const uint64_t FREE_BIT = 0x1;

        // File-backed space
        os::MapRegion _region;

        // Region of persistent memory
        struct RegionHeader;
        RegionHeader * const _pm;

        // Offset from the region's base where objects start
        unsigned _alloc_offset;

        // Serialize access to the allocator
        //spin_lock_t _lock;

        void acquire_lock();
        void release_lock();

        // Stats
        size_t _num_alloc_calls;
        size_t _num_free_calls;

    public:
        FixedAllocator(const char *db_name, const struct AllocatorInfo &info,
                       bool create);

        // Primary allocator functions; serialized
        void *alloc();
        void free(void *p);

        // Support functions for the node and edge iterators; not serialized
        // (depends on the caller to serialize access)
        void *begin() const;
        const void *end() const;
        void *next(const void *curr) const;
        bool is_free(const void *curr) const;

        uint64_t get_id(const void *obj) const;
        unsigned object_size() const;
    };

    class Allocator : public FixedAllocator {
    public:
        Allocator(const char *db_name, const struct AllocatorInfo &info,
                  bool create)
            : FixedAllocator(db_name, info, create)
            { }
    };
}
