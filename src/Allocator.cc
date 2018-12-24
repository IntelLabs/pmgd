/**
 * @file   Allocator.cc
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
#include <thread>
#include <chrono>
#include <cstdlib>

#include "exception.h"
#include "Allocator.h"
#include "arch.h"
#include "GraphImpl.h"

using namespace PMGD;

// For debug build
const unsigned Allocator::AllocatorLock::ALLOC_BACKOFF_TIME;
const unsigned Allocator::AllocatorLock::FREE_BACKOFF_TIME;

Allocator::Allocator(GraphImpl *db, uint64_t pool_addr, uint64_t pool_size,
                      RegionHeader *hdr, uint32_t instances,
                      CommonParams &params)
    : _pm_base(pool_addr),
      _size(pool_size),
      _hdr(hdr),
      _chunks(pool_addr + CHUNK_SIZE, &hdr->chunks_hdr,
                CHUNK_SIZE, pool_size - CHUNK_SIZE, params),
      _allocators(params.create ? instances : _hdr->num_instances),
      _lock_owners(params.create ? instances : _hdr->num_instances)
{
    // No point creating a transaction if this is just a reload.
    if (params.create) {
        TransactionImpl tx(db, Transaction::ReadWrite);
        create_allocators(instances, params);
        tx.commit();
    }
    else
        setup_allocators();
}

void Allocator::setup_allocators()
{
    // Existing fixed allocator. No RangeSet needed.
    CommonParams c(false, false);
    for (unsigned i = 0; i < _hdr->num_instances; ++i) {
        AllocatorUnit::RegionHeader *unit_hdr = _hdr->allocator_hdrs[i];
        _allocators[i] = new AllocatorUnit(this, unit_hdr->_pm_base, unit_hdr, i, c);
    }
}

void Allocator::create_allocators(unsigned instances, CommonParams &params)
{
    // Each allocator's header can occupy close to 500B in the header. So we
    // cannot afford to store information about 'n' (past 5-6) allocators in
    // the graph header any more. So check if it is possible to store, else
    // use a page from the pool to allocate them and save the pointers in the
    // graph header. Only exception is the first header since the default case
    // is one header.
    // The value of instances is verified at config create time.
    _hdr->num_instances = instances;
    _hdr->allocator_hdrs[0] = &_hdr->allocator_hdr0;
    _allocators[0] = new AllocatorUnit(this, _pm_base, _hdr->allocator_hdrs[0], 0,
                                        params);
    init_allocators(instances, params);
    // This will get flushed to PM outside in the caller
}

void Allocator::init_allocators(unsigned instances, CommonParams &params)
{
    // Assume that the caller has already created a valid transaction.
    for (unsigned i = 1; i < instances; ++i) {
        // Get a new header from allocator0. We know it is not being used at
        // this time.
        AllocatorUnit::RegionHeader *hdr =
                (AllocatorUnit::RegionHeader *)_allocators[0]->alloc(sizeof(AllocatorUnit::RegionHeader));
        _hdr->allocator_hdrs[i] = hdr;
        _allocators[i] = new AllocatorUnit(this, (uint64_t)_chunks.alloc(), hdr, i,
                                            params);
    }
}

Allocator *Allocator::get_main_allocator(Graph &db)
{
    return &db._impl->allocator();
}

Allocator::~Allocator()
{
    for (int i = 0; i < _allocators.size(); ++i)
        delete _allocators[i];
}

int Allocator::get_allocator()
{
    TransactionImpl *tx = TransactionImpl::get_tx();
    for (unsigned i = 0; i < AllocatorLock::NUM_ATTEMPTS; ++i) {
        // Try all of them till we get a free allocator.
        // We could keep a lock index to start just after the last assigned
        // allocator but there shouldn't be that many allocators. So the loop
        // is short. And it saves an atomic inc for the lock index.
        for (int alloc_id = 0; alloc_id < _allocators.size(); ++alloc_id) {
            if (_lock_owners[alloc_id].try_lock(tx))
                return alloc_id;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(AllocatorLock::ALLOC_BACKOFF_TIME));
    }
    throw PMGDException(LockTimeout);
}

void *Allocator::alloc(size_t size)
{
    TransactionImpl *tx = TransactionImpl::get_tx();
    AllocatorUnit *allocator = NULL;
    int alloc_id;

    if ( (alloc_id = tx->get_allocator()) == -1) {
        alloc_id = get_allocator();
        tx->set_allocator(alloc_id);
    }

    allocator = _allocators[alloc_id];
    return allocator->alloc(size);
}

void *Allocator::alloc_chunk(unsigned num_contiguous)
{
    // This call also takes care of adding to the callback list
    // for unlocking when the TX commits
    _chunks_lock_owner.lock(TransactionImpl::get_tx());
    return _chunks.alloc(num_contiguous);
}

// The caller should hold a lock for this.
void Allocator::free_chunk(uint64_t chunk_base, unsigned num_contiguous)
{
    _chunks.free((void *)chunk_base, num_contiguous);
}

void Allocator::lock_allocators(const PerAllocatorFreeList &alloc_ids)
{
    TransactionImpl *tx = TransactionImpl::get_tx();

    for (auto it = alloc_ids.begin(); it != alloc_ids.end(); ++it) {
        int alloc_id = it->first;
        assert(alloc_id >= 0 && alloc_id < _allocators.size());
        _lock_owners[alloc_id].lock(tx);
    }
}

void Allocator::free(void *addr, size_t size)
{
    if (addr == NULL || size <= 0)
        return;

    // Ensure some correctness.
    assert(addr >= (void *)_pm_base && addr < (void *)(_pm_base + _size));

    TransactionImpl *tx = TransactionImpl::get_tx();

    int alloc_id = AllocatorUnit::get_alloc_id(addr, size);

    // The delayed_free function adds information about this free and its
    // corresponding allocator in a transaction local list for free at commit.
    // This is race free assuming 1:1 mapping between thread and TX.
    MultiAllocatorFreeCallback::
        delayed_free(tx, alloc_id, this, AllocatorUnit::free_info_t{addr, size});
}

void Allocator::clean_free_list(TransactionImpl *tx,
                                PerAllocatorFreeList &free_list)
{
    // Get all the locks first to make sure we don't do any changes
    // prior to locking and we can still timeout.
    // Also, make sure there are items to free before acquiring any
    // locks. The chunks lock could get quite contended.
    if (free_list.size() <= 0)
        return;
    _chunks_lock_owner.lock(tx);
    lock_allocators(free_list);
    for (auto it = free_list.begin(); it != free_list.end(); ++it) {
        AllocatorUnit *allocator = _allocators[it->first];
        allocator->clean_free_list(tx, it->second);
    }
}

bool Allocator::AllocatorLock::try_lock(TransactionImpl *tx)
{
    TransactionImpl *old_owner = _owner_tx;
    if (old_owner == tx)
        return true;
    if (old_owner == NULL && cmpxchg(_owner_tx, old_owner, tx)) {
        // Make sure this is unlocked at TX commit/abort. Not during.
        tx->register_finalize_callback(this, AllocatorUnlockCallback(this));
        return true;
    }
    return false;
}

void Allocator::AllocatorLock::lock(TransactionImpl *tx)
{
    for (unsigned i = 0; i < NUM_ATTEMPTS; ++i) {
        if (try_lock(tx))
            return;
        std::this_thread::sleep_for(std::chrono::microseconds(ALLOC_BACKOFF_TIME));
    }
    throw PMGDException(LockTimeout);
}

void Allocator::AllocatorLock::release(TransactionImpl *tx)
{
    // This goes on the release list only if the TX
    // acquired this.
    assert (_owner_tx == tx);
    _owner_tx = NULL;
}

void MultiAllocatorFreeCallback::add(int alloc_id, AllocatorUnit::free_info_t s)
{
    int id = alloc_id;

    // alloc_id = -1 implies a chunk allocation which really goes to main
    // and there is no way to identify some specific allocator. But there
    // is a proper free function in ChunkAllocator which is independent of
    // the alloc_id really. So assign some AllocatorUnit that has already been
    // identified for freeing and let it free the
    // allocation. It will anyway go to the ChunkAllocator which will then
    // do the right free with proper rounding up etc. If there weren't any
    // allocators used so far, then use the 0th allocator since it
    // always exists regardless of parallelism.
    if (id < 0) {
        auto it = _free_list.begin();
        if (it != _free_list.end())
            id = it->first;
        else
            id = 0;
    }

    // Will insert the id if not present.
    std::list<AllocatorUnit::free_info_t> &list = _free_list[id];
    list.push_back(s);
}

void MultiAllocatorFreeCallback::delayed_free(TransactionImpl *tx, int alloc_id,
                                 Allocator *allocator, AllocatorUnit::free_info_t s)
{
    auto *f = tx->lookup_commit_callback(allocator);
    if (f == NULL) {
        tx->register_commit_callback(allocator, MultiAllocatorFreeCallback(allocator));

        // The callback object is copied when it is registered,
        // so we have to call lookup again to get a pointer to
        // the stored object.
        f = tx->lookup_commit_callback(allocator);
    }

    auto *cb = f->target<MultiAllocatorFreeCallback>();
    cb->add(alloc_id, s);
}

uint64_t Allocator::used_bytes() const
{
    uint64_t used_bytes = 0;

    // For FixSize Allocator
    for (unsigned i = 0; i < _hdr->num_instances; ++i)
        used_bytes += _allocators[i]->used_bytes();

    used_bytes += _chunks.used_bytes();

    return used_bytes;
}

unsigned Allocator::occupancy() const
{
    uint64_t reserved_bytes = _chunks.used_bytes() + CHUNK_SIZE;
    return 100 * reserved_bytes / (_chunks.region_size() + CHUNK_SIZE);
}

unsigned Allocator::health() const
{
    unsigned health = 0;
    for (unsigned i = 0; i < _hdr->num_instances; ++i)
        health += _allocators[i]->health();
    return health / _hdr->num_instances;
}
