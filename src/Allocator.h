#pragma once

#include <stddef.h>
#include <stdint.h>
#include "TransactionImpl.h"
#include "compiler.h"
#include "FixedAllocator.h"

namespace Jarvis {
    /**
     *  Generic allocator
     *
     *  This class should be used for graph components other than nodes,
     *  edges and string table.
     *
     *  This encapsulates a few commonly used fixed size allocators and the
     *  rest of alloc/free requests go to the variable allocator.
     *
     *  This object lives in DRAM with pointers to data in the graph structure
     */
    class Allocator
    {
    public:
        struct RegionHeader {
            uint64_t free;
        };

    private:
        RegionHeader *_hdr;
        uint64_t _pool_end;

    public:
        static const uint64_t CHUNK_SIZE = 0x200000;

        Allocator(uint64_t pool_addr, uint64_t pool_size,
                  RegionHeader *hdr, bool create)
            : _hdr(hdr), _pool_end(pool_addr + pool_size)
        {
            if (create)
                _hdr->free = pool_addr;
            else
                assert(_hdr->free <= _pool_end);
        }

        void *alloc(size_t size)
        {
            TransactionImpl *tx = TransactionImpl::get_tx();
            uint64_t r = _hdr->free;

            if ((size & (size - 1)) == 0)
                r = (r + size - 1) & ~uint64_t(size - 1);
            else if ((size & (8 - 1)) == 0)
                r = (r + 8 - 1) & ~uint64_t(8 - 1);

            assert(r + size <= _pool_end);

            tx->write(&_hdr->free, r + size);

            return (void *)r;
        }


        void free(void *addr, size_t size) { }
    };

}
