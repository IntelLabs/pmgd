#pragma once

#include <list>
#include <stddef.h>
#include <stdint.h>
#include <unordered_set>
#include <map>
#include "FixedAllocator.h"
#include "TransactionImpl.h"
#include "compiler.h"

namespace Jarvis {
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
    class Allocator
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

                FreeFormChunk(unsigned used = 0);
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
            Allocator &_allocator;

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

        public:
            VariableAllocator(const VariableAllocator &) = delete;
            void operator=(const VariableAllocator &) = delete;
            VariableAllocator(Allocator &allocator, RegionHeader *hdr, bool create);
            void *alloc(size_t size);
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

            // Store a reference to the main allocator for times that we need
            // a new pool
            Allocator &_allocator;

            // DRAM variables for quick access. Unlike other helper allocators,
            // we maintain headers in this pool since that is how we can get to
            // the required FixedAllocator. We do keep only those FixedAllocator
            // headers that still have space.
            std::map<uint64_t, FixedAllocatorInfo *> _fa_pools;
            RegionHeader *_last_hdr_scanned;  // Needed to extend the linked list.

            FixedAllocatorInfo *add_new_pool();
        public:
            FlexFixedAllocator(const FlexFixedAllocator &) = delete;
            void operator=(const FlexFixedAllocator &) = delete;

            FlexFixedAllocator(uint64_t pool_addr, RegionHeader *hdr_addr,
                        unsigned object_size, uint64_t pool_size,
                        Allocator &allocator, bool create);

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
                uint32_t free_spots;     // Total free spots in the chunk
                // Avoid bitmap search when possible.
                uint32_t next_index;     // Next free index within bitmask
                uint32_t occupants[];    // Bitmask to indicate filled spots
                // No need to store the size of chunk or object size
                // since the access allocator structure takes care of accessing
                // the correct sized entities.

                FixedChunk(unsigned obj_size, unsigned bitmap_ints);
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

            // Store a reference to the allocator for requesting new small chunks.
            // Manage small chunks within the 2MB space, one pool at a time.
            // There will always be just one active one each time since that
            // will be used until all the 4K chunks have been exhausted. Then
            // it will just be a used space like the other 2MB pages.
            FlexFixedAllocator &_allocator;

            // We need an available list, in DRAM for for quick access.
            // PM can have just one list. We add to these lists as we traverse
            // during allocations. The filled chunks are removed from DRAM tracking
            // until they have some free space made available.
            std::unordered_set<FixedChunk *> _free_chunks;
            FixedChunk *_chunk_to_scan;      // Point into the FixedAllocator.
            FixedChunk *_last_chunk_scanned; // Needed to extend the linked list

        public:
            FixSizeAllocator(const FixSizeAllocator &) = delete;
            void operator=(const FixSizeAllocator &) = delete;
            FixSizeAllocator(FlexFixedAllocator &allocator, RegionHeader *hdr,
                              unsigned obj_size, bool create);
            void *alloc();
            void free(void *addr);

            uint64_t used_bytes() const;
        };

        class ChunkAllocator
        {
            static const unsigned THRESHOLD = CHUNK_SIZE -
                    (VariableAllocator::HEADER_SIZE + VariableAllocator::MIN_ALLOC_BYTES);

            // Store a reference to the main allocator for times that we need
            // a new pool
            Allocator &_allocator;

        public:
            ChunkAllocator(const ChunkAllocator &) = delete;
            void operator=(const ChunkAllocator &) = delete;
            ChunkAllocator(Allocator &allocator) : _allocator(allocator) {}
            bool is_borderline(size_t sz);
            void *alloc(size_t size);
            void free(void *addr, size_t size);
        };

    public:
        static const unsigned NUM_FIXED_SIZES = 6;
        static unsigned constexpr fixed_sizes[] = { 16, 24, 32, 40, 48, 64};

        static_assert(ARRAY_SIZEOF(fixed_sizes) == NUM_FIXED_SIZES,
                        "mismatch in number of fixed sizes");

        /**
        * Allocator's header
        * In PM, stored in the Graph region to avoid using allocator pool for
        * a header.
        */
        struct RegionHeader {
            // To manage >=2MB chunks
            FixedAllocator::RegionHeader chunks_hdr;

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

        uint64_t *_pm_base;  // Start of PM space
        size_t _size; // Total size for the allocator to manage

        // Manage the 2MB chunks as a FixedAllocator with header in the
        // graph region.
        FixedAllocator _chunks;

        // Internal helper allocators
        VariableAllocator _freeform_allocator;

        FlexFixedAllocator _small_chunks;

        // Instantiate one fix size allocator per size
        FixSizeAllocator *_fixsize_allocator[NUM_FIXED_SIZES];

        ChunkAllocator _chunk_allocator;

        // For use by internal allocators exclusively
        // free_chunk only called at commit time.
        void *alloc_chunk() { return _chunks.alloc(); }
        void *alloc_chunk(unsigned num_contiguous)
          { return _chunks.alloc(num_contiguous); }
        void free_chunk(uint64_t chunk_base, unsigned num_contiguous = 1)
          { _chunks.free(reinterpret_cast<void *>(chunk_base), num_contiguous); }

        unsigned is_fixed(size_t size);

        // We want to make sure the flex fixed allocator can request its header
        // space from the free form allocator. So provide a private function that
        // only it can access.
        void *alloc_free_form(size_t size) { return _freeform_allocator.alloc(size); }
        void free_free_form(void *addr, size_t size) { _freeform_allocator.free(addr, size); }

        friend class AllocatorCallback<Allocator, free_info_t>;
        void clean_free_list(TransactionImpl *tx, const std::list<free_info_t> &list);

    public:
        Allocator(const Allocator &) = delete;
        void operator=(const Allocator &) = delete;
        Allocator(uint64_t pool_addr, uint64_t pool_size,
                    RegionHeader *hdr, bool create);
        void *alloc(size_t size);
        void free(void *addr, size_t size);

        uint64_t region_size() const
            { return _chunks.region_size() + CHUNK_SIZE; }

        uint64_t used_bytes() const;
        unsigned occupancy() const;
        unsigned health() const;
    };

}
