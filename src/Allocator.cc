#include <stddef.h>
#include <assert.h>
#include "exception.h"
#include "os.h"
#include "GraphConfig.h"
#include "allocator.h"
#include "arch.h"

using namespace Jarvis;


Allocator::Allocator(const uint64_t region_addr, int count,
                     const uint64_t allocator_offsets[],
                     const AllocatorInfo fixed_allocator_info[],
                     bool create)
    : _fixed_allocators(count)
{
    for (int i = 0; i < count; ++i)
        if (create)
            _fixed_allocators[i] = new FixedAllocator(region_addr + fixed_allocator_info[i].offset, fixed_allocator_info[i].object_size, fixed_allocator_info[i].pool_size, true);
        else
            _fixed_allocators[i] = new FixedAllocator(region_addr + allocator_offsets[i], 0, 0, false);
}


unsigned Allocator::find_alloc_index(size_t size)
{
    if (size <= _fixed_allocators[0]->object_size())
        return 0;
    unsigned index = bsr(size) - 4; // First power is 2^4 ==> first fixed alloc size
    return ( (size & (size - 1) ) ? (index + 1) : index);
}

void *Allocator::alloc(size_t size)
{
    // TODO This restriction will go away when we have the variable
    // allocator
    if (size > _fixed_allocators[_fixed_allocators.size() - 1]->object_size())
        throw Exception(not_implemented);
    unsigned index = find_alloc_index(size);
    return _fixed_allocators[index]->alloc();
}

void Allocator::free(void *addr, size_t size)
{
    // TODO This restriction will go away when we have the variable
    // allocator
    assert(size <= _fixed_allocators[_fixed_allocators.size() - 1]->object_size());
    unsigned index = find_alloc_index(size);
    return _fixed_allocators[index]->free(addr);
}
