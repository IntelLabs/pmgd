#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Jarvis {
    typedef uint32_t TransactionId;

    const int MAX_TRANSACTIONS = 64;
    const int TRANSACTION_TABLE_SIZE = MAX_TRANSACTIONS*64;

    const uint64_t JOURNAL_EXTENT = 2*1024*1024; //2MB
    const uint64_t JOURNAL_SIZE = JOURNAL_EXTENT*MAX_TRANSACTIONS;

    const int TRANSACTION_REGION_SIZE = TRANSACTION_TABLE_SIZE + JOURNAL_SIZE;

    struct TransactionHandle {
        TransactionId id;
        int index;
        void *jbegin;
        void *jend;

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

        void reset_table(void);
        void recover(void);
        void *tx_jbegin(int index);
        void *tx_jend(int index);

    public:
        TransactionManager(uint64_t region_addr, uint64_t region_size, bool create);

        TransactionHandle alloc_transaction();
        void free_transaction(const TransactionHandle &);
    };
};
