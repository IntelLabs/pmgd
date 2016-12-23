#include <stddef.h>
#include <assert.h>

#include "exception.h"
#include "Allocator.h"
#include "TransactionImpl.h"

using namespace Jarvis;

Allocator::VariableAllocator::VariableAllocator(Allocator &allocator,
                     RegionHeader *hdr, bool create)
    : _allocator(allocator)
{
    if (create)
        hdr->start_chunk = NULL;

    _chunk_to_scan = hdr->start_chunk;
    _last_chunk_scanned = NULL;
    _hdr = hdr;
}

void Allocator::VariableAllocator::FreeFormChunk::find_max_cont_space()
{
    // Have to compute it all over again because we have no idea how
    // the free list got changed before this and since we do not
    // store the index of the current contiguous spot, there is no
    // quick way to know that. Maybe this could be DRAM-optimized
    // if struct FreeFormChunk was re-defined ***.
    // OR is it worth removing the index here? ***
    uint32_t space = 0;
    uint32_t offset = free_list;
    while (offset != 0) {
        free_spot_t *free_spot = compute_addr(offset);
        uint32_t sz_free = free_spot->size;
        if (sz_free > space)
            space = sz_free;
        offset = free_spot->next;
    }
    if (max_cont_space != space) {
        TransactionImpl *tx = TransactionImpl::get_tx();
        tx->write(&max_cont_space, space);
    }
}

void *Allocator::VariableAllocator::FreeFormChunk::alloc(size_t sz)
{
    if (sz > max_cont_space)
        return NULL;
    void *addr = NULL;
    free_spot_t *prev_spot = reinterpret_cast<free_spot_t *>(&free_list);
    uint32_t offset = free_list;

    TransactionImpl *tx = TransactionImpl::get_tx();

    while (true) {
        assert(offset != 0);
        free_spot_t *free_spot = compute_addr(offset);
        uint32_t sz_free = free_spot->size;
        if (sz_free >= sz) {

            // Enough space to create a new free spot
            if (sz_free - sz >= MIN_ALLOC_BYTES) {
                // Allocate space at the end so the free list doesn't
                // have to change.
                uint32_t new_offset = offset + (sz_free - sz);
                addr = compute_addr(new_offset);
                // But we do need to update the size.
                tx->write(&free_spot->size, (uint32_t)(sz_free - sz));
            }
            else {   // This is where we might have some permanently wasted bytes
                addr = compute_addr(offset);

                // We need to find the next free spot and change the
                // free list accordingly.
                offset = free_spot->next;   // first byte at free spot was the next free
                tx->write(&prev_spot->next, offset);
                // No changes to size cause that block remains untouched.

                // Log first 8B of the address being returned to the user
                // since that contained our free list information.
                tx->log(free_spot, sizeof(free_spot_t));
            }
            free_space -= sz;
            if (sz_free == max_cont_space)
                find_max_cont_space();
            return addr;
        } // If the current free spot doesn't have enough room, traverse
        else {
            prev_spot = free_spot;
            // The next offset is stored at the current free spot.
            // But the size of the next spot will be at the spot.
            offset = free_spot->next;
        }
    }
}

Allocator::VariableAllocator::FreeFormChunk::FreeFormChunk(unsigned used)
{
    next_chunk = NULL;
    free_space = CHUNK_SIZE - HEADER_SIZE - used;
    free_list = sizeof(FreeFormChunk);

    free_spot_t *free_spot = compute_addr(free_list);
    free_spot->next = 0;
    free_spot->size = free_space;
    max_cont_space = free_space;
    TransactionImpl::flush_range(this, sizeof(FreeFormChunk) + sizeof(free_spot_t));
}

void *Allocator::VariableAllocator::alloc_large(size_t sz)
{
    // In this case, we always have to make a new chunk allocation
    // Round up to 2MB boundary
    size_t tot_size = (sz + CHUNK_SIZE - 1) & ~(CHUNK_SIZE - 1);
    unsigned num_chunks = tot_size / CHUNK_SIZE;
    unsigned used = sz - (num_chunks - 1) * CHUNK_SIZE;

    FreeFormChunk *dst_chunk = new (_allocator.alloc_chunk(num_chunks)) FreeFormChunk(used);

    void *addr = dst_chunk->compute_addr(CHUNK_SIZE - used);

    // Since this doesn't follow the regular sequence of allocations,
    // make sure the blocks have been scanned to set the pointers
    // correctly. ***
    while (_chunk_to_scan != NULL) {
        // This chunk must not be in the DRAM lists since we go
        // in a sequence.
        // For later scans
        if (_chunk_to_scan->has_space())
            _free_chunks.insert(_chunk_to_scan);
        _last_chunk_scanned = _chunk_to_scan;
        _chunk_to_scan = _chunk_to_scan->next_chunk;
    }

    TransactionImpl *tx = TransactionImpl::get_tx();

    // First free form allocation.
    if (_hdr->start_chunk == NULL)
        tx->write(&_hdr->start_chunk, dst_chunk);
    else
        tx->write(&_last_chunk_scanned->next_chunk, dst_chunk);
    _last_chunk_scanned = dst_chunk;

    // For later scans
    // Since we allocate here only in non-borderline cases,
    // there will always be at least 8B.
    _free_chunks.insert(dst_chunk);

    return addr;
}

void *Allocator::VariableAllocator::alloc(size_t sz)
{
    void *addr = NULL;

    if (sz > CHUNK_SIZE)
        return alloc_large(sz);

    // Check the vector first.
    for (FreeFormChunk *chunk : _free_chunks) {
        addr = chunk->alloc(sz);

        if (addr != NULL) {
            if (!chunk->has_space()) {
                // Make sure this chunk no longer appears in the available list.
                _free_chunks.erase(chunk);
            }
            return addr;
        }
    }

    TransactionImpl *tx = TransactionImpl::get_tx();

    while (_chunk_to_scan != NULL) {
        // This chunk must not be in the DRAM lists since we go
        // in a sequence.
        addr = _chunk_to_scan->alloc(sz);

        // For later scans
        if (_chunk_to_scan->has_space())
            _free_chunks.insert(_chunk_to_scan);
        _last_chunk_scanned = _chunk_to_scan;
        _chunk_to_scan = _chunk_to_scan->next_chunk;
        if (addr != NULL)
            return addr;
    }

    // Reached null while looking for addrs. So all others scanned.
    // If it comes out here, no luck allocating.
    FreeFormChunk *dst_chunk = new (_allocator.alloc_chunk()) FreeFormChunk;

    // First free form allocation.
    if (_hdr->start_chunk == NULL)
        tx->write(&_hdr->start_chunk, dst_chunk);
    else
        tx->write(&_last_chunk_scanned->next_chunk, dst_chunk);
    _last_chunk_scanned = dst_chunk;
    addr = dst_chunk->alloc(sz);

    // For later scans
    if (dst_chunk->has_space())
        _free_chunks.insert(dst_chunk);

    // At this point, since we come here only if the allocation fits
    // in the 2MB chunk, we should have a valid address.
    assert(addr != NULL);

    return addr;
}

void Allocator::VariableAllocator::FreeFormChunk::free(void *addr, size_t sz)
{
    TransactionImpl *tx = TransactionImpl::get_tx();

    tx->log_range(&free_space, &max_cont_space);
    unsigned addr_idx = reinterpret_cast<uint64_t>(addr) - reinterpret_cast<uint64_t>(this);
    free_spot_t *free_spot;
    if (free_list == 0) {  // Indicates a previously full chunk
        free_list = addr_idx;
        free_spot = compute_addr(free_list);
        tx->log(free_spot, sizeof(free_spot_t));
        free_spot->next = 0;
        free_spot->size = sz;
        max_cont_space = sz;
    }
    else if (addr_idx + sz == free_list) {  // Easy coalescing
        free_spot = compute_addr(free_list);
        free_list = addr_idx;
        uint32_t next = free_spot->next;
        uint32_t new_size = free_spot->size + sz;
        free_spot = compute_addr(free_list);
        tx->log(free_spot, sizeof(free_spot_t));
        free_spot->next = next;
        free_spot->size = new_size;
        if (new_size > max_cont_space)
            max_cont_space = new_size;
    }
    else { // Just do a simple return for later use
        // Construct the free list
        free_spot = compute_addr(addr_idx);
        tx->log(free_spot, sizeof(free_spot_t));
        free_spot->next = free_list;
        free_spot->size = sz;
        free_list = addr_idx;
        if (sz > max_cont_space)
            max_cont_space = sz;
    }
    free_space += sz;
}

void Allocator::VariableAllocator::free(void *addr, size_t sz)
{
    static const uint64_t PAGE_MASK = ~(CHUNK_SIZE - 1);
    uint64_t chunk_base = reinterpret_cast<uint64_t>(addr) & PAGE_MASK;

    if (sz > CHUNK_SIZE) {
        size_t tot_size = (sz + CHUNK_SIZE - 1) & ~(CHUNK_SIZE - 1);
        unsigned num_chunks = tot_size / CHUNK_SIZE;

        // Free chunks past the first 2MB boundary
        _allocator.free_chunk(chunk_base + CHUNK_SIZE, num_chunks - 1);

        sz = sz - (num_chunks - 1) * CHUNK_SIZE;
    }

    FreeFormChunk *dst_chunk = reinterpret_cast<FreeFormChunk *>(chunk_base);
    unsigned space = dst_chunk->free_list;
    dst_chunk->free(addr, sz);

    // This was off the available list
    if (space == 0 && dst_chunk->free_space < CHUNK_SIZE - HEADER_SIZE)
        _free_chunks.insert(dst_chunk);

    if (dst_chunk->free_space == CHUNK_SIZE - HEADER_SIZE) {
        _free_chunks.erase(dst_chunk);

        TransactionImpl *tx = TransactionImpl::get_tx();
        // Need to update the implicit list
        // *** Consider doubly linked list here?
        FreeFormChunk *temp = _hdr->start_chunk;
        if (temp == dst_chunk) {
            if (_chunk_to_scan == dst_chunk)
                _chunk_to_scan = NULL;
            tx->log(_hdr, sizeof(RegionHeader));
            _hdr->start_chunk = dst_chunk->next_chunk;
        }
        else {
            FreeFormChunk *prev = temp;
            while(temp != NULL) {
                if (temp == dst_chunk) {
                    if (_chunk_to_scan == dst_chunk)
                        _chunk_to_scan = prev;
                    tx->log(&prev->next_chunk, sizeof(prev->next_chunk));
                    prev->next_chunk = dst_chunk->next_chunk;  // works even if only 1 chunk
                    break;
                }
                prev = temp;
                temp = temp->next_chunk;
            }
        }
        _allocator.free_chunk(chunk_base);
    }
}

uint64_t Allocator::VariableAllocator::reserved_bytes() const
{
    if (_hdr == NULL)
        return 0;

    uint64_t chunk_counter = 0;

    FreeFormChunk *curr = _hdr->start_chunk;

    // Traverse List to count chunks
    while (curr != NULL) {
        ++chunk_counter;
        curr = curr->next_chunk;
    }

    return chunk_counter * CHUNK_SIZE;
}

// This method is almost a duplicate of reserved_bytes(),
// but it is implemented this way to avoid doing that iteration
// over the chunks twice.
uint64_t Allocator::VariableAllocator::used_bytes() const
{
    if (_hdr == NULL)
        return 0;

    uint64_t free_space     = 0;
    size_t   chunk_counter = 0;

    FreeFormChunk *curr = _hdr->start_chunk;

    while (curr != NULL) {
        free_space += curr->free_space;
        ++chunk_counter;
        curr = curr->next_chunk;
    }

    return chunk_counter * CHUNK_SIZE - free_space;
}
