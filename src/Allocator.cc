#include <stddef.h>
#include <assert.h>

#include "exception.h"
#include "Allocator.h"

using namespace Jarvis;

constexpr unsigned Allocator::fixed_sizes[];

Allocator::Allocator(uint64_t pool_addr, uint64_t pool_size,
                      RegionHeader *hdr, bool create)
    : _pm_base(reinterpret_cast<uint64_t *>(pool_addr)),
      _size(pool_size),
      _chunks(pool_addr + CHUNK_SIZE, &hdr->chunks_hdr,
                CHUNK_SIZE, pool_size - CHUNK_SIZE, create),
      _freeform_allocator(*this, &hdr->freeform_hdr, create),
      _small_chunks(pool_addr, &hdr->flex_hdr, FixSizeAllocator::SMALL_CHUNK_SIZE,
                CHUNK_SIZE, *this, create),
      _chunk_allocator(*this)
{
    for (unsigned i = 0; i < NUM_FIXED_SIZES; ++i) {
        _fixsize_allocator[i] = new FixSizeAllocator(_small_chunks,
                                      &hdr->fixsize_hdr[i],
                                      fixed_sizes[i], create);
    }
    // This will get flushed to PM outside in the caller
}

unsigned Allocator::is_fixed(size_t size)
{
    for (unsigned i = 0; i < NUM_FIXED_SIZES; ++i) {
        if (size == fixed_sizes[i])
            return i;
    }
    return NUM_FIXED_SIZES;
}

void *Allocator::alloc(size_t size)
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

void Allocator::free(void *addr, size_t size)
{
    if (addr == NULL)
        return;

    // Ensure some correctness.
    assert(addr >= (void *)_pm_base && addr < (void *)((uint64_t)_pm_base + _size));

    if (size < VariableAllocator::MIN_ALLOC_BYTES)
        size = VariableAllocator::MIN_ALLOC_BYTES;

    TransactionImpl *tx = TransactionImpl::get_tx();

    AllocatorCallback<Allocator, free_info_t>::
        delayed_free(tx, this, free_info_t{addr, size});
}

void Allocator::clean_free_list(TransactionImpl *tx, const std::list<free_info_t> &list)
{
    for (auto s : list) {
        unsigned alloc_idx = is_fixed(s.size);

        if (alloc_idx < NUM_FIXED_SIZES) // not for fixed chunk
            _fixsize_allocator[alloc_idx]->free(s.addr);
        else if (_chunk_allocator.is_borderline(s.size))
            _chunk_allocator.free(s.addr, s.size);
        else
            _freeform_allocator.free(s.addr, s.size);
    }
}
