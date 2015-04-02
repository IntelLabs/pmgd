/*
 * TODOs
 * - Write random data in regions in debug mode
 * - signature checking?
 * - throw an exception instead of returning NULL
 */

#include <stddef.h>
#include <assert.h>

#include "exception.h"
#include "allocator.h"
#include "TransactionImpl.h"

using namespace Jarvis;

/**
 * Allocator's header
 *
 * Header of the allocator located at the beginning of the region
 * it manages.  Uses types of known sizes instead of types such as
 * 'void *' to control layout.
 */
struct FixedAllocator::RegionHeader {
    // Keep following fields together for easy logging
    uint64_t *tail_ptr;
    uint64_t *free_ptr;              ///< Beginning of free list
    // Stats
    int64_t num_allocated;
    uint64_t max_addr;               ///< tail_ptr < max_addr (always)
    uint32_t size;                   ///< Object size
};


FixedAllocator::FixedAllocator(uint64_t pool_addr,
                               unsigned object_size, uint64_t pool_size,
                               bool create)
    : _pm(reinterpret_cast<RegionHeader *>(pool_addr))
{
#define ALLOC_OFFSET(sz) ((sizeof(RegionHeader) + (sz) - 1) & ~((sz) - 1))

    if (create) {
        // Object size must be a power of 2 and at least 8 bytes.
        assert(object_size >= sizeof(uint64_t));
        assert(!(object_size & (object_size - 1)));

        // Start allocation at a natural boundary.
        _alloc_offset = ALLOC_OFFSET(object_size);

        _pm->free_ptr = NULL;
        _pm->tail_ptr = (uint64_t *)(pool_addr + _alloc_offset);
        _pm->size = object_size;
        _pm->max_addr = pool_addr + pool_size;
        _pm->num_allocated = 0;

        TransactionImpl::flush_range(_pm, sizeof(*_pm));
    }
    else
        _alloc_offset = ALLOC_OFFSET(_pm->size);
}

uint64_t FixedAllocator::get_id(const void *obj) const
{
    return (((uint64_t)obj - (uint64_t)begin()) / _pm->size) + 1;
}

unsigned FixedAllocator::object_size() const
{
    return _pm->size;
}

void *FixedAllocator::alloc()
{
    TransactionImpl *tx = TransactionImpl::get_tx();

    tx->log_range(&_pm->tail_ptr, &_pm->num_allocated);

    uint64_t *p;
    if (_pm->free_ptr != NULL) {
        /* We found an object on the free list */
        p = _pm->free_ptr;

        // Log the 8-byte freelist pointer in the allocated object.
        tx->log(p, sizeof(uint64_t));

        *p &= ~FREE_BIT;
        _pm->free_ptr = (uint64_t *)*_pm->free_ptr;
    }
    else
    {
        if (((uint64_t)_pm->tail_ptr + _pm->size) > _pm->max_addr)
            throw Exception(bad_alloc);

        /* Free list exhausted, we are growing our pool of objects by one */
        p = _pm->tail_ptr;
        _pm->tail_ptr = (uint64_t *)((uint64_t)_pm->tail_ptr + _pm->size);
    }

    _pm->num_allocated++;

    return p;
}

/**
 * Free an object
 *
 * A simple implementation that inserts the freed object at the
 * beginning of the free list.
 */
void FixedAllocator::free(void *p)
{
    TransactionImpl *tx = TransactionImpl::get_tx();

    tx->log(p, sizeof(uint64_t));

    void *&free_list = tx->register_commit_callback(this, clean_free_list);
    *(uint64_t *)p = (uint64_t)free_list;
    free_list = p;
}

void FixedAllocator::clean_free_list(TransactionImpl *tx, void *obj, void *free_list)
{
    FixedAllocator *This = (FixedAllocator *)obj;
    tx->log_range(&This->_pm->free_ptr, &This->_pm->num_allocated);
    void *free_ptr = This->_pm->free_ptr;
    int64_t num_allocated = This->_pm->num_allocated;

    void *q;
    for (void *p = free_list; p != NULL; p = q) {
        q = *(void **)p;
        *(uint64_t *)p = (uint64_t)free_ptr | FREE_BIT;
        free_ptr = p;
        num_allocated--;
    }

    This->_pm->free_ptr = (uint64_t *)free_ptr;
    This->_pm->num_allocated = num_allocated;
}

void *FixedAllocator::begin() const
{
    return (void *)((uint64_t)_pm + _alloc_offset);
}

const void *FixedAllocator::end() const
{
    return _pm->tail_ptr;
}

void *FixedAllocator::next(const void *curr) const
{
    return (void *)((uint64_t)curr + _pm->size);
}

bool FixedAllocator::is_free(const void *curr) const
{
    return *(uint64_t *)curr & FREE_BIT;
}
