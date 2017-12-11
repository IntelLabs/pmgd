/**
 * @file   Allocator.h
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

#include <list>
#include <vector>
#include <map>

#include "TransactionImpl.h"
#include "GraphConfig.h"
#include "AllocatorUnit.h"

namespace PMGD {
    class GraphImpl;

    /**
     *  Group to request and free from assigned generic allocator(s)
     */
    class Allocator
    {
        typedef std::map<int, std::list<AllocatorUnit::free_info_t>>
                                                          PerAllocatorFreeList;

    public:
        static const uint64_t CHUNK_SIZE = 0x200000;     // in bytes

        /**
        * Allocator's header
        * In PM, stored in the Graph region to avoid using allocator pool for
        * a header.
        */
        struct RegionHeader {
            // To manage >=2MB chunks
            FixedAllocator::RegionHeader chunks_hdr;

            // Number of allocator instances created at graph create.
            uint32_t num_instances;

            // Keep space for the first one so it can be used to get
            // more space when transactions actually start using the
            // multiple instances.
            AllocatorUnit::RegionHeader allocator_hdr0;
            
            // Region headers for the 'n' allocators
            AllocatorUnit::RegionHeader *allocator_hdrs[];
        };

    private:
        struct AllocatorLock {
            // These parameters can best be determined by instrumentation of various
            // experiments. We will adjust these as we learn more.
            static const unsigned NUM_ATTEMPTS = 3; // how often to retry when all busy
            static const unsigned ALLOC_BACKOFF_TIME = 1000; // in microseconds
            static const unsigned FREE_BACKOFF_TIME = 10000; // in microseconds

            TransactionImpl *_owner_tx;

            AllocatorLock() : _owner_tx(NULL) {}
            void lock(TransactionImpl *curr_tx);
            bool try_lock(TransactionImpl *curr_tx);
            void release(TransactionImpl *tx);
        };

        const uint64_t _pm_base;  // Start of PM space
        const size_t _size; // Total size for the allocator to manage
      
        RegionHeader * const _hdr;  // Pointer into the allocator space in graph struct

        // Manage the 2MB chunks as a FixedAllocator with header in the
        // graph region.
        FixedAllocator _chunks;

        // Manage all the 'n' allocator instances.
        std::vector<AllocatorUnit *> _allocators;

        friend class AllocatorUnlockCallback;

        // Store the holder transaction per allocator in case the same
        // transaction wants to re-acquire it.
        AllocatorLock _chunks_lock_owner;
        std::vector<AllocatorLock> _lock_owners;

        // Use at graph create time.
        void create_allocators(unsigned instances, CommonParams &params);

        // Call this when graph is created and more than one allocators are required.
        void init_allocators(unsigned instances, CommonParams &params);

        // Use at graph reload time to setup from existing headers.
        void setup_allocators();

        // Call at first allocation time for a transaction.
        int get_allocator();

        // This function can be used when freeing.
        // The allocator ids need to be ordered. Hence input as set.
        void lock_allocators(const PerAllocatorFreeList &alloc_ids);

        friend class MultiAllocatorFreeCallback;
        void clean_free_list(TransactionImpl *tx,
                        PerAllocatorFreeList &free_list);

        friend class AllocatorUnit;
        void *alloc_chunk(unsigned num_contiguous = 1);

        // Free will only be called at commit time.
        // _chunks still needs to be locked but no sub-transaction required.
        // If failure happens during commit, then log entries for free shoul,d
        // remain with other 
        void free_chunk(uint64_t chunk_base, unsigned num_contiguous = 1);

        // Define this constructor for unit tests. This assumes that
        // the tests already have a valid transaction object. Only the friend
        // tests here will be able to use it.
        friend class AllocTest;

        // AllocTest requires the start base for all its computations. So
        // let it find that out.
        uint64_t get_start_addr() { return _pm_base; }

        friend class AllocAbortTest;
        friend class MTAllocTest;
        friend class AvlTreeTest;
        friend class MTAvlTreeTest;
        friend class EdgeIndexTest;
        friend class ListTest;
        friend class ChunkListTest;
        static Allocator *get_main_allocator(Graph &db);
    public:
        Allocator(const Allocator &) = delete;
        void operator=(const Allocator &) = delete;

        // Need to pass the GraphImpl ptr to allow for a transaction here
        // and succeed in allocating from the allocator0.
        Allocator(GraphImpl *db, uint64_t pool_addr, uint64_t pool_size,
                      RegionHeader *hdr, uint32_t instances, CommonParams &params);
        ~Allocator();

        void *alloc(size_t size);
        void free(void *addr, size_t size);

        // For stats
        uint64_t region_size() const
            { return _chunks.region_size() + CHUNK_SIZE; }
        uint64_t used_bytes() const;
        unsigned occupancy() const;
        unsigned health() const;
    };

    class MultiAllocatorFreeCallback
    {
        typedef Allocator::PerAllocatorFreeList PerAllocatorFreeList;

        Allocator *_allocator;

        // With multiple allocators, we need a per allocator list.
        // Hence the allocator id.
        PerAllocatorFreeList _free_list;

        void add(int alloc_id, AllocatorUnit::free_info_t s);

    public:
        MultiAllocatorFreeCallback(Allocator *a) : _allocator(a) { }

        void operator()(TransactionImpl *tx)
        {
            _allocator->clean_free_list(tx, _free_list);
        }

        static void delayed_free(TransactionImpl *tx, int alloc_id,
                                 Allocator *allocator, AllocatorUnit::free_info_t s);
    };

    class AllocatorUnlockCallback
    {
        Allocator::AllocatorLock *_alloc_lock;

    public:
        AllocatorUnlockCallback(Allocator::AllocatorLock *al) : _alloc_lock(al)
          { }
        void operator()(TransactionImpl *tx) { _alloc_lock->release(tx); }
    };
}
