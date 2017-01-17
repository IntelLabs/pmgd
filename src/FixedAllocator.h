/**
 * @file   FixedAllocator.h
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

#include <stddef.h>
#include <stdint.h>
#include "TransactionImpl.h"
#include "GraphConfig.h"

namespace PMGD {
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

        friend class AllocatorCallback;
        void clean_free_list(TransactionImpl *tx, const std::list<void *> &list);

    public:
        FixedAllocator(const FixedAllocator &) = delete;
        void operator=(const FixedAllocator &) = delete;

        FixedAllocator(uint64_t pool_addr, RegionHeader *hdr_addr,
                               uint32_t object_size, uint64_t pool_size,
                               CommonParams &params);

        FixedAllocator(uint64_t pool_addr,
                               uint32_t object_size, uint64_t pool_size,
                               CommonParams &params);

        // Primary allocator functions; serialized
        void *alloc();
        void free(void *p);

        // Support for contiguous multi-object allocations and commit time
        // free. These are only used by the Allocator. This free must only
        // be called at commit time.
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

        int64_t num_allocated() const
          { return _pm->num_allocated; }

        static int64_t num_allocated(RegionHeader *hdr)
          { return hdr->num_allocated; }

        uint64_t get_id(const void *obj) const
          { return (((uint64_t)obj - (uint64_t)begin()) / _pm->size) + 1; }

        uint32_t object_size() const
          { return _pm->size; }

        uint64_t used_bytes() const
          { return _pm->size * (uint64_t)_pm->num_allocated; }

        uint64_t region_size() const
          { return _pm->max_addr - _pool_addr; }

        unsigned occupancy() const;
        unsigned health() const;
    };

    class AllocatorCallback
    {
        FixedAllocator *_allocator;
        std::list<void *> _list;
    public:
        AllocatorCallback(FixedAllocator *a) : _allocator(a) { }

        void operator()(TransactionImpl *tx)
            { _allocator->clean_free_list(tx, _list); }

        void add(void *s) { _list.push_front(s); }

        static void delayed_free(TransactionImpl *tx, FixedAllocator *allocator,
                                 void *s)
        {
            auto *f = tx->lookup_commit_callback(allocator);
            if (f == NULL) {
                tx->register_commit_callback(allocator, AllocatorCallback(allocator));

                // The callback object is copied when it is registered,
                // so we have to call lookup again to get a pointer to
                // the stored object.
                f = tx->lookup_commit_callback(allocator);
            }

            auto *cb = f->template target<AllocatorCallback>();
            cb->add(s);
        }
    };
}
