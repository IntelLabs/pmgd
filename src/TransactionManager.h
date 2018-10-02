/**
 * @file   TransactionManager.h
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

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <emmintrin.h>
#include <immintrin.h>
#include "arch.h"
#include "os.h"
#include "GraphConfig.h"

namespace PMGD {
    // TransactionId is never reset and should not roll-over.
    // A 63-bit transaction ID supports a billion transactions
    // per second for 100 years. The high bit is used to indicate
    // that a transaction is in use.
    typedef uint64_t TransactionId;

    struct TransactionHandle {
        TransactionId id;
        int index;
        void *jbegin;
        void *jend;

        TransactionHandle() : id(-1), index(-1), jbegin(NULL), jend(NULL) {}

        TransactionHandle(TransactionId a, int b, void *c, void *d)
            : id(a), index(b), jbegin(c), jend(d)
            {}
    };

    // Transaction table entry (64B)
    struct alignas(64) TransactionHdr {
        // Transaction is in use when high bit of Transaction ID is set
        static const uint64_t ACTIVE = 1ull << 63;
        TransactionId tx_id;
        void *jbegin;
        void *jend;
    };

    class TransactionManager {
        // Transaction table in PM
        TransactionHdr *_tx_table;

        // Journal in PM
        void *_journal_addr;

        TransactionId _cur_tx_id;
        int _max_transactions;
        size_t _extent_size;
        int _max_extents;

        void reset_table(bool msync_needed, RangeSet &pending_commits);
        void recover(bool read_only, bool msync_needed, RangeSet &pending_commits);
        void *tx_jbegin(int index);
        void *tx_jend(int index);

    public:
        TransactionManager(const TransactionManager &) = delete;
        void operator=(const TransactionManager &) = delete;

        TransactionManager(uint64_t transaction_table_addr,
                           uint64_t transaction_table_size,
                           uint64_t journal_addr,
                           uint64_t journal_size,
                           CommonParams &params);

        TransactionHandle alloc_transaction(bool read_only, bool msync_needed, RangeSet &);
        void free_transaction(const TransactionHandle &, bool msync_needed, RangeSet &);

        // Need a neutral spot to declare the following functions
        // that handle persistence via PM way or msync way. In case
        // of msync, the caller decides based on Graph create time
        // flags if some msync action is needed or not.
        static inline void flush(void *addr, bool msync_needed, RangeSet &pending_commits)
        {
#ifdef PM  // Means there is persistent memory
            clflush(addr);
#else   // MSYNC
            if (msync_needed)
                os::flush(addr, pending_commits);
#endif
        }

        static inline void commit(bool msync_commit, RangeSet &pending_commits)
        {
#ifdef PM
            persistent_barrier();
#else   // MSYNC
            if (msync_commit)
                os::commit(pending_commits);
#endif
        }
    };
};
