#include <stddef.h>
#include <assert.h>

#include "exception.h"
#include "Allocator.h"
#include "TransactionImpl.h"

using namespace Jarvis;

void *Allocator::ChunkAllocator::alloc(size_t sz)
{
    // In this case, we always have to make a new chunk allocation
    // Round up to 2MB boundary
    size_t tot_size = (sz + CHUNK_SIZE - 1) & ~(CHUNK_SIZE - 1);
    unsigned  num_chunks = tot_size / CHUNK_SIZE;
    return _allocator.alloc_chunk(num_chunks);
}

void Allocator::ChunkAllocator::free(void *addr, size_t sz)
{
    uint64_t chunk_base = reinterpret_cast<uint64_t>(addr);
    assert(chunk_base % CHUNK_SIZE == 0);
    size_t tot_size = (sz + CHUNK_SIZE - 1) & ~(CHUNK_SIZE - 1);
    unsigned num_chunks = tot_size / CHUNK_SIZE;
    _allocator.free_chunk(chunk_base, num_chunks);
}

bool Allocator::ChunkAllocator::is_borderline(size_t sz)
{
    size_t mod = sz % CHUNK_SIZE;
    return (mod == 0 || mod > THRESHOLD);
}
