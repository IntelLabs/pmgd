/**
 * @file   FlexFixedAllocator.cc
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
using namespace std;

Allocator::FlexFixedAllocator::FlexFixedAllocator(uint64_t pool_addr,
                      RegionHeader *hdr_addr,
                      unsigned object_size, uint64_t pool_size,
                      Allocator &allocator, bool create)
    : _pm(hdr_addr),
      _obj_size(object_size), _pool_size(pool_size),
      _max_objs_per_pool(pool_size / object_size),
      _allocator(allocator)
{
    // Make sure we have a well-aligned pool_addr.
    assert((pool_addr & (CHUNK_SIZE - 1)) == 0);

    // Since the header allocation for the first pool is
    // different than all other pools, create the first FixedAllocator
    // here or instantiate with an existing one. The other helper
    // allocators just start with NULL until they are required. Since
    // we use FlexFixedAllocator for FixSizeAllocator which allocates
    // data structures for our code, and most likely there will at least
    // be one index (loader.id), this allocation won't be a waste.
    int64_t num_allocated;
    if (create) {
        _pm->pool_base = pool_addr;
        _pm->next_pool_hdr = NULL;
        num_allocated = 0;
    }
    else
        num_allocated = FixedAllocator::num_allocated(&_pm->fa_hdr);

    _last_hdr_scanned = _pm;
    if (num_allocated < _max_objs_per_pool) {
        FixedAllocator *fa = new FixedAllocator(pool_addr, &_pm->fa_hdr,
                            _obj_size, _pool_size, create);
        _fa_pools.insert(pair<uint64_t,FixedAllocatorInfo*>(pool_addr,
                            new FixedAllocatorInfo{fa, _pm,
                            NULL, fa->num_allocated()}) );
    }
}

void *Allocator::FlexFixedAllocator::alloc()
{
    void *addr = NULL;
    auto it = _fa_pools.begin();

    if (it != _fa_pools.end()) {
        FixedAllocatorInfo *fa_info = it->second;
        addr = fa_info->fa->alloc();
        fa_info->num_allocated++;

        // If the fa is in the list, it should have space.
        assert(addr != NULL);

        if (fa_info->num_allocated == _max_objs_per_pool) {
            // Make sure this fa no longer appears in the available list.
            _fa_pools.erase(it);
            delete fa_info;
        }
        return addr;
    }

    RegionHeader *hdr;
    while ( (hdr = _last_hdr_scanned->next_pool_hdr) != NULL) {
        RegionHeader *prev = _last_hdr_scanned;
        _last_hdr_scanned = hdr;

        // This chunk must not be in the DRAM lists since we go
        // in a sequence.
        int64_t num_allocated = FixedAllocator::num_allocated(&hdr->fa_hdr);
        if (num_allocated < _max_objs_per_pool) {
            FixedAllocator *fa = new FixedAllocator(hdr->pool_base, &hdr->fa_hdr,
                                    _obj_size, _pool_size, false);
            addr = fa->alloc();
            num_allocated++;

            // For later scans
            if (num_allocated < _max_objs_per_pool) {
                _fa_pools.insert(pair<uint64_t,FixedAllocatorInfo*>(hdr->pool_base,
                                    new FixedAllocatorInfo{fa, hdr,
                                    prev, num_allocated}) );
            }
            else
                delete fa;

            return addr;
        }
    }

    // If it comes out here, no luck allocating.
    FixedAllocatorInfo *fa_info = add_new_pool();
    addr = fa_info->fa->alloc();
    fa_info->num_allocated++;

    return addr;
}

Allocator::FlexFixedAllocator::FixedAllocatorInfo *Allocator::FlexFixedAllocator::add_new_pool()
{
    // If it comes out here, no luck allocating.
    uint64_t pool_addr = reinterpret_cast<uint64_t>(_allocator.alloc_chunk());

    // We also need space for the header. Easiest is to get it
    // from the free form chunk, even if it is a fixed size header
    // otherwise we will have a chicken and egg problem.
    RegionHeader *hdr =
        new (_allocator.alloc_free_form(sizeof(RegionHeader))) RegionHeader;

    hdr->pool_base = pool_addr;
    FixedAllocator *fa = new FixedAllocator(pool_addr, &(hdr->fa_hdr),
                            _obj_size, _pool_size, true);
    hdr->next_pool_hdr = NULL;
    TransactionImpl::flush_range(hdr, sizeof(*hdr) - sizeof(hdr->fa_hdr));

    TransactionImpl *tx = TransactionImpl::get_tx();
    tx->write(&_last_hdr_scanned->next_pool_hdr, hdr);

    FixedAllocatorInfo *fa_info = new FixedAllocatorInfo{fa,
                                            hdr, _last_hdr_scanned, 0};
    _fa_pools.insert(pair<uint64_t, FixedAllocatorInfo*>(pool_addr, fa_info));
    _last_hdr_scanned = hdr;

    return fa_info;
}

void Allocator::FlexFixedAllocator::free(void *addr)
{
    TransactionImpl *tx = TransactionImpl::get_tx();
    static const uint64_t PAGE_MASK = ~(CHUNK_SIZE - 1);
    uint64_t pool_base = reinterpret_cast<uint64_t>(addr) & PAGE_MASK;

    // We update the DRAM list as needed. So it is possible that some
    // fixed allocator might not be present. In that case, traverse the
    // PM list.
    RegionHeader *hdr, *prev = NULL;
    FixedAllocator *fa = NULL;
    int64_t num_allocated;
    FixedAllocatorInfo *fa_info = NULL;
    auto it = _fa_pools.find(pool_base);

    if (it != _fa_pools.end()) {
        fa_info = it->second;
        hdr = fa_info->hdr;
        prev = fa_info->prev;
        fa = fa_info->fa;
        fa_info->num_allocated--;
        num_allocated = fa_info->num_allocated;
    }
    else {
        hdr = _pm;
        while (hdr != NULL) {
            if (hdr->pool_base == pool_base)
                break;
            prev = hdr;
            hdr = hdr->next_pool_hdr;
        }
        assert(hdr != NULL);

        // Since we haven't actually freed anything in PM, just count 1 less
        num_allocated = FixedAllocator::num_allocated(&(hdr->fa_hdr)) - 1;
    }

    if (num_allocated == 0 && hdr != _pm) {
        // No need to free the 4K chunk anymore since the entire
        // 2M chunk is going to be freed.

        // Remove this header from the PM linked list of headers
        // prev cannot be null since hdr is not _pm
        tx->write(&prev->next_pool_hdr, hdr->next_pool_hdr);

        if (hdr->next_pool_hdr != NULL) {
            uint64_t next_pool = hdr->next_pool_hdr->pool_base;

            // Correct the prev pointer for some other entry.
            it = _fa_pools.find(next_pool);
            if (it != _fa_pools.end()) {
                FixedAllocatorInfo *fa_next = it->second;
                fa_next->prev = prev;
            }
        }

        if (_last_hdr_scanned == hdr)
            _last_hdr_scanned = prev;

        // The free_chunk function already rounds down to base.
        _allocator.free_chunk(pool_base);
        _allocator.free_free_form(hdr, sizeof(RegionHeader));

        // Remove the DRAM entry too
        if (fa_info != NULL) {
            _fa_pools.erase(pool_base);
            delete fa_info;
        }
    }
    else {
        if (fa == NULL) {
            fa = new FixedAllocator(pool_base, &(hdr->fa_hdr),
                                    _obj_size, _pool_size, false);
        }
        fa->free(addr, 1);
        if (fa_info == NULL) {
            // Cache this info in case the map didn't already contain it.
            // Will help optimize future frees.
            _fa_pools.insert(pair<uint64_t, FixedAllocatorInfo*>(pool_base,
                                              new FixedAllocatorInfo{fa, hdr,
                                              prev, num_allocated}));
        }
    }
}

int64_t Allocator::FlexFixedAllocator::num_allocated() const
{
    int64_t counter = 0;

    RegionHeader *curr = _pm;

    while(curr != NULL) {
        ++counter;
        curr = curr->next_pool_hdr;
    }

    return counter;
}
