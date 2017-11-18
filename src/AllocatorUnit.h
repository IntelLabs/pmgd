/**
 * @file   AllocatorUnit.h
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
#include <stddef.h>
#include <stdint.h>
#include <unordered_map>
#include <unordered_set>
#include "FixedAllocator.h"
#include "TransactionImpl.h"
#include "compiler.h"
#include "GraphConfig.h"

namespace PMGD {
    class Allocator;

    // This callback is for restoring DRAM state when allocs push
    // some regions out of the available lists. But then if these
    // allocs do not persist due to transaction abort, those removed
    // regions must be made available again. There is no such problem
    // to deal when freeing because the actual free calls are not made
    // till the transaction tries to commit.
    template<typename A>
    class AllocatorAbortCallback
    {
        A *_allocator;

        std::unordered_map<void *, bool> _to_fix;

        void add(void *chunk, bool remove)
        {
            // If the same chunk goes through multiple changes in a TX,
            // and an abort happens, rollback will go reverse and leave
            // the very first change in. So only add this once to the map
            // with the very first action.
            auto it = _to_fix.find(chunk);
            if (it == _to_fix.end())
                _to_fix[chunk] = remove;
        }

    public:
        AllocatorAbortCallback(A *a)
            : _allocator(a)
          { }

        void operator()(TransactionImpl *tx)
        {
            for (auto it = _to_fix.begin(); it != _to_fix.end(); ++it) {
                if (it->second)
                    _allocator->remove_dram_chunk(it->first);
                else
                    _allocator->restore_dram_chunk(it->first);
            }
        }

        static void restore_dram_state(TransactionImpl *tx, A *allocator,
                                        void *chunk, bool remove = false)
        {
            auto *f = tx->lookup_abort_callback(allocator);
            if (f == NULL) {
                tx->register_abort_callback(allocator, AllocatorAbortCallback(allocator));

                // The callback object is copied when it is registered,
                // so we have to call lookup again to get a pointer to
                // the stored object.
                f = tx->lookup_abort_callback(allocator);
            }

            auto *cb = f->template target<AllocatorAbortCallback>();
            cb->add(chunk, remove);
        }
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
     *  This object lives in DRAM with pointers to data in the graph structure
     */
    class AllocatorUnit
    {
    public:
        static const uint64_t CHUNK_SIZE = 0x200000;     // in bytes

    private:
        // Helper allocator classes
        class VariableAllocator
        {
            struct FreeFormChunk {
                // We use first 8B in every free spot to store
                // the next pointer for the free list in this chunk
                // and the size of the block we are looking at. So
                // use this struct to manipulate that space.
                struct free_spot_t {
                    uint32_t next;
                    uint32_t size;
                };

                FreeFormChunk *next_chunk; // Pointer to next free form chunk
                // The free_space member could help us figure how much wasted space
                // there is in case the free spots don't total up to this number.
                uint32_t free_space;  // total free space in the chunk
                uint32_t free_list;  // Offset of the first free spot
                // max_cont_space allows us to avoid searching in a chunk that could not
                // accommodate a request anyway.
                uint32_t max_cont_space;  // Max contiguous space in this chunk
                // Allocator id to find which free list a free request goes to.
                uint32_t my_id;

                FreeFormChunk(TransactionImpl *tx, unsigned alloc_id, unsigned used = 0);
                bool has_space() { return max_cont_space >= MIN_ALLOC_BYTES; }
                void *alloc(size_t sz);
                void free(void *addr, size_t size);
                void find_max_cont_space();
                free_spot_t *compute_addr(uint64_t offset)
                  { return reinterpret_cast<free_spot_t *>(reinterpret_cast<uint64_t>(this) + offset); }
            };

        public:
            static const unsigned HEADER_SIZE = sizeof(FreeFormChunk);
            static const unsigned MIN_ALLOC_BYTES = 8;

            struct RegionHeader {
                FreeFormChunk *start_chunk;
            };

        private:
            RegionHeader *_hdr;  // Pointer into the allocator space in graph struct

            // Store a reference to the main allocator for times that we need
            // a new pool
            AllocatorUnit &_allocator;

            // Store a DRAM version of the allocator id to set in new chunks.
            uint32_t _my_id;

            // We need an available list, in DRAM for the free
            // form chunks since what might seem like a full chunk for one allocation
            // might have enough room for another, unless it is really full.
            // PM can have just one list. We add to these lists as we traverse
            // during allocations. The filled chunks are removed from DRAM tracking
            // until they have some free space made available.
            // Using an unordered_set instead of simple list to avoid the full list
            // traversal when we move chunks between available and full lists which
            // may or may not turn out to be very common.
            // Using a set makes it random in terms of which chunk is considered
            // first for the allocation but that should be ok or we need a tree.
            std::unordered_set<FreeFormChunk *> _free_chunks;
            FreeFormChunk *_chunk_to_scan;  // Point into the FixedAllocator.
            FreeFormChunk *_last_chunk_scanned;  // Needed to extend the linked list

            // This function assumes that the borderline case has already
            // been handled.
            void *alloc_large(size_t size);

            // This version acquires chunk from main allocator for regular allocs
            FreeFormChunk *alloc_chunk();

            // This version is used by alloc_large
            FreeFormChunk *alloc_chunks(unsigned num, unsigned used);

            friend class AllocatorAbortCallback<VariableAllocator>;
            void restore_dram_chunk(void *chunk);
            void remove_dram_chunk(void *chunk);

        public:
            VariableAllocator(const VariableAllocator &) = delete;
            void operator=(const VariableAllocator &) = delete;
            VariableAllocator(AllocatorUnit &allocator, RegionHeader *hdr, uint32_t alloc_id, bool create);
            void *alloc(size_t size);
            static int get_alloc_id(void *addr)
            {
                static const uint64_t PAGE_MASK = ~(CHUNK_SIZE - 1);
                uint64_t chunk_base = reinterpret_cast<uint64_t>(addr) & PAGE_MASK;
                FreeFormChunk *hdr = reinterpret_cast<FreeFormChunk *>(chunk_base);
                return hdr->my_id;
            }
            void free(void *addr, size_t size);

            uint64_t used_bytes() const;
            uint64_t reserved_bytes() const;

        };

        class FlexFixedAllocator
        {
        public:
            // In order to avoid modifying the existing FixedAllocator, we allocate
            // this header in separate PM area. It will maintain other information
            // relevant to the FlexFixedAllocator to allow us to navigate between
            // various FixedAllocator pools and also have space for the header
            // of the FixedAllocators themselves since we are not using the pool
            // itself for the header.
            struct RegionHeader {
                uint64_t pool_base;
                RegionHeader *next_pool_hdr;
                FixedAllocator::RegionHeader fa_hdr;
            };

        private:
            // Define a structure to keep in DRAM as value of the hashmap
            // that can be used to find a pool's base and its corresponding
            // FixedAllocator pointer for quick free.
            struct FixedAllocatorInfo {
                FixedAllocator *fa;
                // We also need a pointer to the PM header for the fixed
                // allocator since it is no longer inlined
                RegionHeader *hdr;
                // When a fixed allocator pool is about to be freed to the
                // main 2MB allocator, we need to maintain the PM linked
                // list. So we need the previous pointer too.
                RegionHeader *prev;
                // Since the count of freed items is not updated until end
                // of transaction, and we need this information to decide
                // whether the 2MB chunk should be returned to the main
                // allocator, maintain this ourselves.
                int64_t num_allocated;

                ~FixedAllocatorInfo() { if (fa != NULL) delete fa; }
            };

            // Start of the RegionHeader list.
            RegionHeader * const _pm;

            // Variables used repeatedly
            unsigned _obj_size;
            uint64_t _pool_size;
            unsigned _max_objs_per_pool;

            // For msync cases
            bool _msync_needed;

            // Store a reference to the main allocator for times that we need
            // a new pool
            AllocatorUnit &_allocator;

            // DRAM variables for quick access. Unlike other helper allocators,
            // we maintain headers in this pool since that is how we can get to
            // the required FixedAllocator. We do keep only those FixedAllocator
            // headers that still have space.
            std::unordered_map<uint64_t, FixedAllocatorInfo *> _fa_pools;
            RegionHeader *_last_hdr_scanned;  // Needed to extend the linked list.

            FixedAllocatorInfo *add_new_pool();

            friend class AllocatorAbortCallback<FlexFixedAllocator>;
            void restore_dram_chunk(void *chunk)
            { }

            void remove_dram_chunk(void *info);

        public:
            FlexFixedAllocator(const FlexFixedAllocator &) = delete;
            void operator=(const FlexFixedAllocator &) = delete;

            FlexFixedAllocator(uint64_t pool_addr, RegionHeader *hdr_addr,
                        unsigned object_size, uint64_t pool_size,
                        AllocatorUnit &allocator, CommonParams &params);

            // Expects the caller to ensure transaction commit to make
            // the changes here permanent and not dependent on the user TX.
            void *alloc();
            void free(void *addr);

            int64_t num_allocated() const;
            uint64_t reserved_bytes() const
                { return num_allocated() * _obj_size; }

        };

        class FixSizeAllocator
        {
            struct FixedChunk {
                FixedChunk *next_chunk;  // Pointer to next chunk of same size slots
                // Allocator id to find which free list a free request goes to.
                uint32_t my_id;
                uint32_t free_spots;     // Total free spots in the chunk
                // Avoid bitmap search when possible.
                uint32_t next_index;     // Next free index within bitmask
                uint32_t occupants[];    // Bitmask to indicate filled spots
                // No need to store the size of chunk or object size
                // since the access allocator structure takes care of accessing
                // the correct sized entities.

                FixedChunk(unsigned alloc_id, unsigned bitmap_ints, unsigned max_spots);
                void *alloc(unsigned obj_size, unsigned bitmap_ints);
                void free(void *addr, unsigned obj_size, unsigned bitmap_ints);
            };

        public:
            static const uint64_t SMALL_CHUNK_SIZE = 4096;  // in bytes

            struct RegionHeader {
                FixedChunk *start_chunk;
            };

        private:
            static const uint64_t PAGE_MASK = ~(SMALL_CHUNK_SIZE - 1);

            RegionHeader *_hdr;  // Pointer into the allocator space in graph struct

            // Store the object size this allocator is responsible for, for
            // internal computations.
            unsigned _obj_size;
            unsigned _bitmap_ints;
            unsigned _max_spots;   // Just store this to avoid computing repeatedly.

            // Store a reference to the allocator for requesting new small chunks.
            // Manage small chunks within the 2MB space, one pool at a time.
            // There will always be just one active one each time since that
            // will be used until all the 4K chunks have been exhausted. Then
            // it will just be a used space like the other 2MB pages.
            FlexFixedAllocator &_allocator;

            // Store a DRAM version of the allocator id to set in new chunks.
            uint32_t _my_id;

            // We need an available list, in DRAM for for quick access.
            // PM can have just one list. We add to these lists as we traverse
            // during allocations. The filled chunks are removed from DRAM tracking
            // until they have some free space made available.
            std::unordered_set<FixedChunk *> _free_chunks;
            FixedChunk *_chunk_to_scan;      // Point into the FixedAllocator.
            FixedChunk *_last_chunk_scanned; // Needed to extend the linked list

            friend class AllocatorAbortCallback<FixSizeAllocator>;
            void restore_dram_chunk(void *chunk);
            void remove_dram_chunk(void *chunk);

        public:
            FixSizeAllocator(const FixSizeAllocator &) = delete;
            void operator=(const FixSizeAllocator &) = delete;
            FixSizeAllocator(FlexFixedAllocator &allocator, RegionHeader *hdr,
                              unsigned obj_size, uint32_t alloc_id, bool create);
            void *alloc();
            static int get_alloc_id(void *addr)
            {
                uint64_t chunk_base = reinterpret_cast<uint64_t>(addr) & PAGE_MASK;
                FixedChunk *hdr = reinterpret_cast<FixedChunk *>(chunk_base);
                return hdr->my_id;
            }
            void free(void *addr);

            uint64_t used_bytes() const;
        };

        class ChunkAllocator
        {
            static const unsigned THRESHOLD = CHUNK_SIZE -
                    (VariableAllocator::HEADER_SIZE + VariableAllocator::MIN_ALLOC_BYTES);

            // Store a reference to the main allocator for times that we need
            // a new pool
            AllocatorUnit &_allocator;

        public:
            ChunkAllocator(const ChunkAllocator &) = delete;
            void operator=(const ChunkAllocator &) = delete;
            ChunkAllocator(AllocatorUnit &allocator) : _allocator(allocator) {}
            static bool is_borderline(size_t sz);
            void *alloc(size_t size);
            void free(void *addr, size_t size);
        };

    public:
        static const unsigned NUM_FIXED_SIZES = 6;
        static unsigned constexpr fixed_sizes[] = { 16, 24, 32, 40, 48, 64};

        static_assert(ARRAY_SIZEOF(fixed_sizes) == NUM_FIXED_SIZES,
                        "mismatch in number of fixed sizes");

        /**
        * AllocatorUnit's header
        * In PM, stored in the Graph region to avoid using allocator pool for
        * a header.
        */
        struct RegionHeader {
            // AllocatorUnit id to find which free list a free request goes to.
            uint32_t my_id;

            // Need to store the pool address that the flex fixed allocator
            // was initialized with to re-initialize it correctly at restart.
            uint64_t _pm_base;  // Start of PM space

            VariableAllocator::RegionHeader freeform_hdr;

            // To manage 4K chunks within 2MB chunks
            FlexFixedAllocator::RegionHeader flex_hdr;

            FixSizeAllocator::RegionHeader fixsize_hdr[NUM_FIXED_SIZES];
        };

    private:
        // Fields needed to support the delayed free at commit time.
        struct free_info_t {
            void *addr;
            size_t size;
        };

        // Parent allocator to request chunk when out.
        Allocator *_parent;

        // Internal helper allocators
        VariableAllocator _freeform_allocator;

        FlexFixedAllocator _small_chunks;

        // Instantiate one fix size allocator per size
        FixSizeAllocator *_fixsize_allocator[NUM_FIXED_SIZES];

        ChunkAllocator _chunk_allocator;

        static unsigned is_fixed(size_t size);
        static int get_alloc_id(void *addr, size_t size);

        // For use by internal allocators exclusively
        // free_chunk only called at commit time.
        void *alloc_chunk(unsigned num_contiguous = 1);
        void free_chunk(uint64_t chunk_base, unsigned num_contiguous = 1);

        // We want to make sure the flex fixed allocator can request its header
        // space from the free form allocator. So provide a private function that
        // only it can access.
        void *alloc_free_form(size_t size) { return _freeform_allocator.alloc(size); }
        void free_free_form(void *addr, size_t size) { _freeform_allocator.free(addr, size); }

        friend class MultiAllocatorFreeCallback;
        friend class Allocator;
        void clean_free_list(TransactionImpl *tx, const std::list<free_info_t> &list);

    public:
        AllocatorUnit(const AllocatorUnit &) = delete;
        void operator=(const AllocatorUnit &) = delete;
        AllocatorUnit(Allocator *a, uint64_t pool_addr,
                  RegionHeader *hdr, uint32_t alloc_id, CommonParams &params);
        void *alloc(size_t size);

        uint64_t used_bytes() const;
        unsigned health() const;
    };
}
