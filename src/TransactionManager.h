#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Jarvis {
    typedef uint32_t TransactionId;

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
        TransactionId tx_id; // non-zero => in-use
        void *jbegin;
        void *jend;
    };

    class TransactionManager {
        TransactionId _cur_tx_id;

        // Transaction table in PM
        TransactionHdr *_tx_table;

        // Journal in PM
        void *_journal_addr;

        int _max_transactions;
        size_t _extent_size;
        int _max_extents;

        void reset_table(void);
        void recover(void);
        void check_clean(void);
        void *tx_jbegin(int index);
        void *tx_jend(int index);

    public:
        TransactionManager(uint64_t transaction_table_addr,
                           uint64_t transaction_table_size,
                           uint64_t journal_addr,
                           uint64_t journal_size,
                           bool create, bool read_only);

        TransactionHandle alloc_transaction(bool read_only);
        void free_transaction(const TransactionHandle &);
    };
};
