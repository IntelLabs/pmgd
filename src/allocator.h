#pragma once

#include <vector>
#include <stddef.h>
#include <stdint.h>

namespace Jarvis {

    /**
     * Fixed-size allocator parameters
     *
     * Each allocator instance operates on a named region of
     * persistent memory.
     */
    struct AllocatorInfo {
        uint64_t offset;                ///< Offset from start of region
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
        FixedAllocator(const uint64_t region_addr,
                       const struct AllocatorInfo &info, bool create);

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

    /** 
     *  Generic allocator
     *
     *  This class should be used for graph components other than nodes,
     *  edges and string table.
     *
     *  This encapsulates a few commonly used fixed size allocators and the
     *  rest of alloc/free requests go to the variable allocator.
     *
     *  This object lives in DRAM. Since it doesn't know the number of 
     *  fixed allocators that will be created in graph.cc, we use a 
     *  vector here
     */
    class Allocator {
        std::vector<FixedAllocator *> _fixed_allocators;

        unsigned find_alloc_index(size_t size);

    public:
        Allocator(const uint64_t region_addr,
                  const struct AllocatorInfo fixed_info[], int count, bool create)
                    : _fixed_allocators(count)
        {
            for (int i = 0; i < count; ++i) { 
                _fixed_allocators[i] = new FixedAllocator(region_addr, fixed_info[i], create);
            }
        }
        void *alloc(size_t size);
        void free(void *addr, size_t size);
    };
}
