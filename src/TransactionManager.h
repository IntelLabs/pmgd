#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Jarvis {
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

        void reset_table();
        void recover(bool read_only);
        void *tx_jbegin(int index);
        void *tx_jend(int index);

    public:
        TransactionManager(const TransactionManager &) = delete;
        void operator=(const TransactionManager &) = delete;

        TransactionManager(uint64_t transaction_table_addr,
                           uint64_t transaction_table_size,
                           uint64_t journal_addr,
                           uint64_t journal_size,
                           bool create, bool read_only);

        TransactionHandle alloc_transaction(bool read_only);
        void free_transaction(const TransactionHandle &);
    };
};
