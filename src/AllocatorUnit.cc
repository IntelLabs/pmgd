/**
 * @file   AllocatorUnit.cc
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
#include "AllocatorUnit.h"
#include "Allocator.h"

using namespace PMGD;

constexpr unsigned AllocatorUnit::fixed_sizes[];

AllocatorUnit::AllocatorUnit(Allocator *a, uint64_t pool_addr, RegionHeader *hdr,
                     uint32_t alloc_id, CommonParams &params)
    : _parent(a),
      _freeform_allocator(*this, &hdr->freeform_hdr, alloc_id, params.create),
      _small_chunks(pool_addr, &hdr->flex_hdr, FixSizeAllocator::SMALL_CHUNK_SIZE,
                CHUNK_SIZE, *this, params),
      _chunk_allocator(*this)
{
    if (params.create) {
        hdr->my_id = alloc_id;
        hdr->_pm_base = pool_addr;
    }
    assert(hdr->my_id == alloc_id);

    for (unsigned i = 0; i < NUM_FIXED_SIZES; ++i) {
        _fixsize_allocator[i] = new FixSizeAllocator(_small_chunks,
                                      &hdr->fixsize_hdr[i],
                                      fixed_sizes[i], alloc_id, params.create);
    }
    // This will get flushed to PM outside in the caller
}

unsigned AllocatorUnit::is_fixed(size_t size)
{
    for (unsigned i = 0; i < NUM_FIXED_SIZES; ++i) {
        if (size == fixed_sizes[i])
            return i;
    }
    return NUM_FIXED_SIZES;
}

void *AllocatorUnit::alloc(size_t size)
{
    if (size < VariableAllocator::MIN_ALLOC_BYTES)
        size = VariableAllocator::MIN_ALLOC_BYTES;

    unsigned alloc_idx = is_fixed(size);

    if (alloc_idx < NUM_FIXED_SIZES)  // not for fixed chunk
        return _fixsize_allocator[alloc_idx]->alloc();
    if (_chunk_allocator.is_borderline(size))
        return _chunk_allocator.alloc(size);
    return _freeform_allocator.alloc(size);
}

void *AllocatorUnit::alloc_chunk(unsigned num_contiguous)
{
    return _parent->alloc_chunk(num_contiguous);
}

int AllocatorUnit::get_alloc_id(void *addr, size_t size)
{
    unsigned alloc_idx = is_fixed(size);

    if (alloc_idx < NUM_FIXED_SIZES)
        return FixSizeAllocator::get_alloc_id(addr);
    if (ChunkAllocator::is_borderline(size))
        return -1;
    return VariableAllocator::get_alloc_id(addr);
}

void AllocatorUnit::clean_free_list(TransactionImpl *tx, const std::list<free_info_t> &list)
{
    for (auto s : list) {
        unsigned alloc_idx = is_fixed(s.size);

        if (alloc_idx < NUM_FIXED_SIZES) // not for fixed chunk
            _fixsize_allocator[alloc_idx]->free(s.addr);
        else if (_chunk_allocator.is_borderline(s.size))
            _chunk_allocator.free(s.addr, s.size);
        else {
            _freeform_allocator.free(s.addr,
                (s.size < VariableAllocator::MIN_ALLOC_BYTES ? VariableAllocator::MIN_ALLOC_BYTES : s.size));
        }
    }
}

void AllocatorUnit::free_chunk(uint64_t chunk_base, unsigned num_contiguous)
{
    _parent->free_chunk(chunk_base, num_contiguous);
}

uint64_t AllocatorUnit::used_bytes() const
{
    uint64_t used_bytes = 0;

    // For FixSize Allocator
    for (unsigned i = 0; i < NUM_FIXED_SIZES; ++i) {
        used_bytes += _fixsize_allocator[i]->used_bytes();
    }

    // For Variable Allocator
    used_bytes += _freeform_allocator.used_bytes();

    // For ChunkAllocator
    // Consider every byte in the ChunkAllocator as used
    used_bytes += _freeform_allocator.reserved_bytes() +
                  _small_chunks.reserved_bytes();

    return used_bytes;
}

unsigned AllocatorUnit::health() const
{
    uint64_t total_used_bytes = 0;

    // For FixSize Allocator
    for (unsigned i = 0; i < NUM_FIXED_SIZES; ++i) {
        total_used_bytes += _fixsize_allocator[i]->used_bytes();
    }

    // For Variable Allocator
    total_used_bytes += _freeform_allocator.used_bytes();

    uint64_t total_bytes = _small_chunks.reserved_bytes() +
                           _freeform_allocator.reserved_bytes();

    if (total_bytes == 0)
        return 100;
    else
        return 100 * total_used_bytes / total_bytes;
}
