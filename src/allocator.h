#pragma once

#include <vector>
#include <stddef.h>
#include <stdint.h>
#include "TransactionImpl.h"

namespace Jarvis {

    struct AllocatorInfo;

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

        // Stats
        size_t _num_alloc_calls;
        size_t _num_free_calls;

        static void clean_free_list(TransactionImpl *tx, void *obj, void *list);

    public:
        FixedAllocator(const FixedAllocator &) = delete;
        void operator=(const FixedAllocator &) = delete;

        FixedAllocator(uint64_t pool_addr, unsigned object_size,
                       uint64_t pool_size, bool create);

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
        // This constructor uses allocator_offsets[] if create is false
        // or fixed_allocator_info[] if create is true.
        Allocator(const uint64_t region_addr, int count,
                  const uint64_t allocator_offsets[],
                  const AllocatorInfo fixed_allocator_info[],
                  bool create);
        void *alloc(size_t size);
        void free(void *addr, size_t size);
    };
}
