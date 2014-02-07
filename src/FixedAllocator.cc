/*
 * TODOs
 * - Write random data in regions in debug mode
 * - Where do calls to fences go?
 * - Remove TX_* macros when transactions are implemented
 * - lock init in constructor actually?
 * - signature checking?
 * - throw an exception instead of returning NULL
 * - rearrange fields so that we minimize calls to TX infrastructure
 *       Better to log a little more data if it means fewer calls
 * - fix TX code and review with Sanjay, for instance:
 *       If transaction A allocates node A and then transaction B
 *       allocates node B and then transaction B commits and then
 *       transaction A aborts, the unwind of transaction A will free
 *       node B.  The same problem happens with either the free list
 *       or the tail_ptr.
 */

#include <stddef.h>
#include <iostream>
#include <assert.h>

#include "exception.h"
#include "os.h"
#include "allocator.h"

// See TODOs above
#define TX_write(_pm, sz)
#define TX_write3(_pm, val, sz)
#define TX_log(ptr, size)

/**
 * Allocator's header
 *
 * Header of the allocator located at the beginning of the region
 * it manages.  Uses types of known sizes instead of types such as
 * 'void *' to control layout.
 */
struct Jarvis::FixedAllocator::RegionHeader {
    uint64_t *tail_ptr;
    uint64_t *free_ptr;              ///< Beginning of free list 
    uint32_t size;                   ///< Object size
    uint32_t zero;                   ///< Zero region before use
    uint64_t max_addr;               ///< tail_ptr < max_addr (always)

    // Stats
    int64_t num_allocated;
};

Jarvis::FixedAllocator::FixedAllocator(const char *db_name,
                                       const AllocatorInfo &info, bool create)
    : _region(db_name, info.name, info.addr, info.len, create, create),
      _pm(reinterpret_cast<RegionHeader *>(info.addr))
{
    // Start allocation at a natural boundary.  Assume object size is
    // a power of 2 of at least 8 bytes.
    assert(info.size > sizeof(uint64_t));
    assert(!(info.size & (info.size - 1)));
    _alloc_offset = ((unsigned)sizeof(RegionHeader) + info.size - 1) & ~(info.size - 1);

    if (create) {
        _pm->free_ptr = NULL;
        _pm->tail_ptr = (uint64_t *)(info.addr + _alloc_offset);
        _pm->size = info.size;
        _pm->zero = info.zero;
        _pm->max_addr = info.addr + info.len;
        _pm->num_allocated = 0;

        TX_write(_pm, sz);
    }
}

void *Jarvis::FixedAllocator::alloc()
{
    acquire_lock();

    uint64_t *p;

    if (_pm->free_ptr != NULL) {
        /* We found an object on the free list */

        p = _pm->free_ptr;

        TX_log(_pm->free_ptr, sizeof(_pm->free_ptr));
        TX_log(*p, sizeof(uint64_t));
        TX_log(_pm->num_allocated, sizeof(_pm->num_allocated));

        *p &= ~FREE_BIT;

        _pm->free_ptr = (uint64_t *)*_pm->free_ptr;

        if (_pm->zero)
            TX_write3(p, 0, _pm->size);

        _pm->num_allocated++;

        release_lock();

        return p;
    }
    if (((uint64_t)_pm->tail_ptr + _pm->size) > _pm->max_addr)
        throw e_bad_alloc;

    /* Free list exhausted, we are growing our pool of objects by one */

    p = _pm->tail_ptr;

    TX_log(_pm->tail_ptr, sizeof(_pm->tail_ptr));
    TX_log(_pm->num_allocated, sizeof(_pm->num_allocated));

    _pm->tail_ptr = (uint64_t *)((uint64_t)_pm->tail_ptr + _pm->size);

    if (_pm->zero)
        TX_write3(p, 0, _pm->size);

    _pm->num_allocated++;

    release_lock();

    return p;
}

/**
 * Free an object
 *
 * A simple implementation that inserts the freed object at the
 * beginning of the free list.
 */
void Jarvis::FixedAllocator::free(void *p)
{
    acquire_lock();

    TX_log(_pm->free_ptr, sizeof(_pm->free_ptr));
    TX_log(_pm->num_allocated, sizeof(_pm->num_allocated));

    *(uint64_t *)p = (uint64_t)_pm->free_ptr | FREE_BIT;
    _pm->free_ptr = (uint64_t *)p;
    _pm->num_allocated--;

    release_lock();
}

void *Jarvis::FixedAllocator::begin()
{
    return (void *)((unsigned long)_pm + _alloc_offset);
}

void *Jarvis::FixedAllocator::end()
{
    return _pm->tail_ptr;
}

void *Jarvis::FixedAllocator::next(void *curr)
{
    return (void *)((unsigned long)curr + _pm->size);
}

bool Jarvis::FixedAllocator::is_free(void *curr)
{
    return *(uint64_t *)curr & FREE_BIT;
}

void Jarvis::FixedAllocator::acquire_lock()
{
}

void Jarvis::FixedAllocator::release_lock()
{
}
