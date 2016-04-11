#pragma once

#include <list>
#include <stddef.h>
#include <stdint.h>
#include "AllocatorCallback.h"
#include "FixedAllocator.h"
#include "TransactionImpl.h"

namespace Jarvis {
    struct AllocatorInfo;

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
