#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Jarvis {
    typedef uint32_t TransactionId;

    const int MAX_TRANSACTIONS = 64;
    const int TRANSACTION_TABLE_SIZE = MAX_TRANSACTIONS*64;

    const uint64_t JOURNAL_EXTENT = 2*1024*1024; //2MB
    const uint64_t JOURNAL_SIZE = JOURNAL_EXTENT*MAX_TRANSACTIONS;

    const int TRANSACTION_INFO_SIZE = TRANSACTION_TABLE_SIZE + JOURNAL_SIZE;

    class TransactionHandle {
        TransactionId _id;
        int _index;
        void *_jbegin;
        void *_jend;
    public:
        TransactionHandle(TransactionId id, int index, void *jbegin,
            void *jend):_id(id), _index(index), _jbegin(jbegin), _jend(jend) {}

        ~TransactionHandle() {}

        TransactionId get_tx_id() { return _id; }
        int get_index() { return _index; }
        void *get_jbegin() { return _jbegin;	}
        void *get_jend() { return _jend;	}
    };

    // Transaction table entry (64B)
    struct TransactionHdr {
        TransactionId tx_id; // non-zero => in-use
        uint32_t reserved0;
        void *jbegin;
        void *jend;
        uint64_t reserved1[5];
    };

    class TransactionManager {
        TransactionId _cur_tx_id;

        // Transaction table in PM
        TransactionHdr *_tx_table;

        // Journal in PM
        void *_journal_addr;

        void _reset_table(void);
        void _recover(void);
        void *_tx_jbegin(int index);
        void *_tx_jend(int index);

    public:
		TransactionManager(void *base_addr, bool create);
        ~TransactionManager() {}

        TransactionHandle *alloc_transaction(void);
        void free_transaction(TransactionHandle *);
    };
};

