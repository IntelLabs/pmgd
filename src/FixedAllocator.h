#pragma once

#include <stddef.h>
#include <stdint.h>
#include "TransactionImpl.h"
#include "AllocatorCallback.h"

namespace Jarvis {
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
    public:
        /**
        * Allocator's header
        *
        * Header of the allocator that could be located at the beginning of the
        * region it manages unless a header area is specified.
        * Uses types of known sizes instead of types such as
        * 'int' to control layout.
        */
        struct RegionHeader {
            // Keep following fields together for easy logging
            uint64_t *tail_ptr;
            uint64_t *free_ptr;              ///< Beginning of free list
            // Stats
            int64_t num_allocated;
            uint64_t max_addr;               ///< tail_ptr < max_addr (always)
            uint32_t size;                   ///< Object size
        };

    private:
        static const uint64_t FREE_BIT = 0x1;

        // Region of persistent memory
        RegionHeader * const _pm;
        // In case the header is specified to be at a different location
        // than the beginning of the pool, we need a way to know where the
        // pool starts from
        uint64_t _pool_addr;

        // Offset from the region's base where objects start
        unsigned _alloc_offset;

        // Maintain objects to be freed at commit time, in this list.
        std::list<void *> _free_list;

        // Stats
        size_t _num_alloc_calls;
        size_t _num_free_calls;

        friend class AllocatorCallback<FixedAllocator, void *>;
        void clean_free_list(TransactionImpl *tx, const std::list<void *> &list);

    public:
        FixedAllocator(const FixedAllocator &) = delete;
        void operator=(const FixedAllocator &) = delete;
        
        FixedAllocator(uint64_t pool_addr, RegionHeader *hdr_addr,
                               uint32_t object_size, uint64_t pool_size,
                               bool create);

        FixedAllocator(uint64_t pool_addr,
                               uint32_t object_size, uint64_t pool_size,
                               bool create);

        // Primary allocator functions; serialized
        void *alloc();
        void free(void *p);

        // Support for contiguous multi-object allocations
        void *alloc(unsigned num_contiguous);
        void free(void *p, unsigned num_contiguous);

        // Support functions for the node and edge iterators; not serialized
        // (depends on the caller to serialize access)
        void *begin() const
          { return (void *)((uint64_t)_pool_addr + _alloc_offset); }

        const void *end() const { return _pm->tail_ptr; }

        void *next(const void *curr) const 
          { return (void *)((uint64_t)curr + _pm->size); }

        bool is_free(const void *curr) const
          { return *(uint64_t *)curr & FREE_BIT; }

        const int64_t num_allocated() const { return _pm->num_allocated; }

        static int64_t num_allocated(RegionHeader *hdr)
          { return hdr->num_allocated; }

        uint64_t get_id(const void *obj) const
          { return (((uint64_t)obj - (uint64_t)begin()) / _pm->size) + 1; }

        unsigned object_size() const { return _pm->size; }
    };
}
