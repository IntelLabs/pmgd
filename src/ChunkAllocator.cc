/**
 * @file   ChunkAllocator.cc
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
#include "TransactionImpl.h"

using namespace PMGD;

void *AllocatorUnit::ChunkAllocator::alloc(size_t sz)
{
    // In this case, we always have to make a new chunk allocation
    // Round up to 2MB boundary
    size_t tot_size = (sz + CHUNK_SIZE - 1) & ~(CHUNK_SIZE - 1);
    unsigned  num_chunks = tot_size / CHUNK_SIZE;
    void *addr;

    // Even though this call would cause the main allocator to lock
    // up for the duration of this TX, we cannot put an inner TX
    // since this is a user allocation and if user TX aborts, we don't
    // want this page to be permanently allocated with no trace of it.
    addr = _allocator.alloc_chunk(num_chunks);

    return addr; 
}

void AllocatorUnit::ChunkAllocator::free(void *addr, size_t sz)
{
    uint64_t chunk_base = reinterpret_cast<uint64_t>(addr);
    assert(chunk_base % CHUNK_SIZE == 0);
    size_t tot_size = (sz + CHUNK_SIZE - 1) & ~(CHUNK_SIZE - 1);
    unsigned num_chunks = tot_size / CHUNK_SIZE;
    _allocator.free_chunk(chunk_base, num_chunks);
}

bool AllocatorUnit::ChunkAllocator::is_borderline(size_t sz)
{
    size_t mod = sz % CHUNK_SIZE;
    return (mod == 0 || mod > THRESHOLD);
}
