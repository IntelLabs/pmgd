/**
 * @file   FixSizeAllocator.cc
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

#include <stddef.h>
#include <assert.h>

#include "exception.h"
#include "Allocator.h"
#include "TransactionImpl.h"

using namespace PMGD;

#define ALLOC_OFFSET(bitints,sz) ((sizeof(FixedChunk) + (bitints)*4 + (sz) - 1) & ~((sz) - 1))

Allocator::FixSizeAllocator::FixSizeAllocator(FlexFixedAllocator &allocator,
                                    RegionHeader *hdr,
                                    unsigned obj_size, bool create)
    : _allocator(allocator)
{
    _hdr = hdr;
    _obj_size = obj_size;
    if (create)
        hdr->start_chunk = NULL;

    // Have to find the ceiling using number of bits in the bitmap
    // since that determines the number of bitints needed to store the
    // number of spots for objects in the chunk.
    unsigned bits = _obj_size * 32;
    _bitmap_ints = (SMALL_CHUNK_SIZE + bits - 1) / bits;

    _chunk_to_scan = hdr->start_chunk;
    _last_chunk_scanned = NULL;

    _max_spots = (SMALL_CHUNK_SIZE - ALLOC_OFFSET(_bitmap_ints, _obj_size)) /
                          _obj_size;
}

Allocator::FixSizeAllocator::FixedChunk::FixedChunk(unsigned obj_size,
                                                    unsigned bitmap_ints,
                                                    unsigned max_spots)
{
    next_chunk = NULL;
    free_spots = max_spots;
    next_index = 0;
    for (unsigned i = 0; i < bitmap_ints; ++i)
        occupants[i] = 0;
    // Some bits of the last bitmap entry might be unused. Set them
    // to unavailable.
    unsigned num_entries = 32; // 32 = number of entries per bitmap int
    int main_idx = free_spots / num_entries;
    int sub_idx = free_spots % num_entries;
    uint32_t mask = ~( (1u << sub_idx) - 1);
    occupants[main_idx] |= mask;

    TransactionImpl::flush_range(this, sizeof(*this));
}

void *Allocator::FixSizeAllocator::FixedChunk::alloc(unsigned obj_size,
                                                        unsigned bitmap_ints)
{
    // Next index may point to a free spot in a chunk where there
    // is free space. Make allocs fast.

    assert(free_spots > 0);

    unsigned num_entries = 32;  // number of entries per bitmap int
    unsigned max_spots = (SMALL_CHUNK_SIZE - ALLOC_OFFSET(bitmap_ints, obj_size)) /
                          obj_size;
    unsigned index = next_index;
    unsigned main_idx;
    unsigned sub_idx;
    uint32_t mask;

    while (index <= max_spots) {
        main_idx = index / num_entries;
        sub_idx = index % num_entries;
        mask = 1u << sub_idx;
        if ((occupants[main_idx] & mask) == 0) {
            goto found;
       }
       index++;
    }

    // Restart search at beginning. This is guaranteed to find a spot.
    for (main_idx = 0; main_idx < bitmap_ints; ++main_idx) {
        for (sub_idx = 0; sub_idx < num_entries; ++sub_idx) {
            mask = 1u << sub_idx;
            if ((occupants[main_idx] & mask) == 0) {
                index = main_idx * num_entries + sub_idx;
                goto found;
            }
        }
    }
    assert(0);
    return NULL;

found:
    TransactionImpl *tx = TransactionImpl::get_tx();
    tx->log_range(&free_spots, &next_index);
    // *** Could combine the next one in the range if logging just the first
    // int
    tx->log(&occupants[main_idx], sizeof(uint32_t));
    --free_spots;
    occupants[main_idx] |= mask;
    next_index = index + 1;

    // Compute address for allocation
    return (uint8_t *)this
               + ALLOC_OFFSET(bitmap_ints, obj_size)
               + (obj_size * index);
}

void *Allocator::FixSizeAllocator::alloc()
{
    void *addr = NULL;
    auto it = _free_chunks.begin();

    // Check the vector first.
    if (it != _free_chunks.end()) {
        FixedChunk *dst_chunk = *it;
        addr = dst_chunk->alloc(_obj_size, _bitmap_ints);

        // If the chunk is in the list, it should have space.
        assert(addr != NULL);

        if (dst_chunk->free_spots == 0) {
            // Make sure this chunk no longer appears in the available list.
            _free_chunks.erase(it);
        }
        return addr;
    }

    TransactionImpl *tx = TransactionImpl::get_tx();

    while (_chunk_to_scan != NULL) {
        FixedChunk *dst_chunk = _chunk_to_scan;
        _last_chunk_scanned = _chunk_to_scan;
        _chunk_to_scan = _chunk_to_scan->next_chunk;

        // This chunk must not be in the DRAM lists since we go
        // in a sequence.
        if (dst_chunk->free_spots > 0) {
            addr = dst_chunk->alloc(_obj_size, _bitmap_ints);

            // For later scans
            if (dst_chunk->free_spots > 0)
                _free_chunks.insert(dst_chunk);

            return addr;
        }
    }

    // Reached null while looking for addrs. So all others scanned.
    // If it comes out here, no luck allocating.
    FixedChunk *dst_chunk = new (_allocator.alloc()) FixedChunk(_obj_size,
                                                                _bitmap_ints,
                                                                _max_spots);

    // First free form allocation.
    if (_hdr->start_chunk == NULL)
        tx->write(&_hdr->start_chunk, dst_chunk);
    else
        tx->write(&_last_chunk_scanned->next_chunk, dst_chunk);
    _last_chunk_scanned = dst_chunk;
    addr = dst_chunk->alloc(_obj_size, _bitmap_ints);

    // Since we just allocated an entire chunk for one request,
    // it obviously has space left. So add it to that list.
    _free_chunks.insert(dst_chunk);

    // At this point, since we come here only if the allocation fits
    // in the 2MB chunk, we should have a valid address.
    assert(addr != NULL);

    return addr;
}

void Allocator::FixSizeAllocator::FixedChunk::free(void *addr, unsigned obj_size, unsigned bitmap_ints)
{
    uint64_t chunk_base = reinterpret_cast<uint64_t>(addr) & PAGE_MASK;
    uint64_t alloc_base = chunk_base + ALLOC_OFFSET(bitmap_ints, obj_size);
    assert(addr >= (void *)alloc_base && addr < (void *)(chunk_base + SMALL_CHUNK_SIZE));
    assert((reinterpret_cast<uint64_t>(addr) - alloc_base) % obj_size == 0);
    unsigned addr_idx = (reinterpret_cast<uint64_t>(addr) - alloc_base) / obj_size;

    TransactionImpl *tx = TransactionImpl::get_tx();

    unsigned num_entries = 32; // 32 = number of entries per bitmap int
    int main_idx = addr_idx / num_entries;
    int sub_idx = addr_idx % num_entries;  // bit within the particular bitmap int to mark used.
    // address for allocation
    uint32_t mask = 1u << sub_idx;

    tx->log_range(&free_spots, &next_index);
    tx->log(&occupants[main_idx], sizeof(uint32_t));
    occupants[main_idx] &= ~mask;

    if (free_spots == 0)
        next_index = addr_idx;
    // else leave the next_index untouched.

    ++free_spots;
}

void Allocator::FixSizeAllocator::free(void *addr)
{
    uint64_t chunk_base = reinterpret_cast<uint64_t>(addr) & PAGE_MASK;
    FixedChunk *dst_chunk = reinterpret_cast<FixedChunk *>(chunk_base);

    unsigned space = dst_chunk->free_spots;
    assert(space < _max_spots);
    dst_chunk->free(addr, _obj_size, _bitmap_ints);


    // If this frees up the entire fixed chunk, and this was the only chunk
    // in the 2MB page, return the 2MB page. But only if this isn't the current
    // allocation chunk that the DRAM object uses to search for free space.
    if (dst_chunk->free_spots == _max_spots) {
        _free_chunks.erase(dst_chunk);

        TransactionImpl *tx = TransactionImpl::get_tx();

        // Need to update the implicit list
        // *** Consider doubly linked list here?
        FixedChunk *temp = _hdr->start_chunk, *prev = NULL;
        if (temp == dst_chunk) {
            tx->write(&_hdr->start_chunk, dst_chunk->next_chunk);
        }
        else {
            while(temp != NULL) {
                if (temp == dst_chunk) {
                    tx->write(&prev->next_chunk, dst_chunk->next_chunk);
                    break;
                }
                prev = temp;
                temp = temp->next_chunk;
            }
        }

        if (_last_chunk_scanned == dst_chunk)
            _last_chunk_scanned = prev;
        if (_chunk_to_scan == dst_chunk)
            _chunk_to_scan = dst_chunk->next_chunk;

        _allocator.free(dst_chunk);
    }
    // This was off the available list
    else if (space == 0)
        _free_chunks.insert(dst_chunk);
}

uint64_t Allocator::FixSizeAllocator::used_bytes() const
{
    if (_hdr == NULL)
        return 0;

    uint64_t free_spot_counter = 0;
    uint64_t chunk_counter     = 0;

    FixedChunk *curr = _hdr->start_chunk;

    while (curr != NULL) {
        free_spot_counter += curr->free_spots;
        ++chunk_counter;
        curr = curr->next_chunk;
    }

    uint64_t free_space = free_spot_counter * _obj_size;

    return chunk_counter * SMALL_CHUNK_SIZE - free_space;
}
