
#include <stddef.h>
#include "exception.h"
#include "os.h"
#include "allocator.h"

using namespace Jarvis;

static inline unsigned bsr(uint64_t value)
{
    uint64_t r;
    // Find the index of the highest bit = 1
    asm("bsr %1,%0" : "=r"(r) : "r"(value));
    return (unsigned)r;
}

unsigned Allocator::find_alloc_index(size_t size)
{
    unsigned index = bsr(size) - 4; // First power is 2^4 ==> first fixed alloc size
    return ( (size & (size - 1) ) ? (index + 1) : index);
}

void *Allocator::alloc(size_t size)
{
    // TODO This restriction will go away when we have the variable
    // allocator
    if (size < _fixed_allocators[0]->object_size() ||
        size > _fixed_allocators[_fixed_allocators.size() - 1]->object_size())
        throw Exception(not_implemented);
    unsigned index = find_alloc_index(size);
    return _fixed_allocators[index]->alloc();
}

void Allocator::free(void *addr, size_t size)
{
    // TODO This restriction will go away when we have the variable
    // allocator
    if (size < _fixed_allocators[0]->object_size() ||
        size > _fixed_allocators[_fixed_allocators.size() - 1]->object_size())
        throw Exception(internal_error);
    unsigned index = find_alloc_index(size);
    return _fixed_allocators[index]->free(addr);
}
