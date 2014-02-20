#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stack>

namespace Jarvis {

    class Lock;
    class TransactionHandle;
    struct JournalEntry;

    class TransactionImpl {
        public:
            typedef uint32_t TransactionId;

        private:
            TransactionId _tx_id;
            bool _committed;

            TransactionHandle *_tx_handle;
            JournalEntry *_jbegin;
            JournalEntry *_jend;
            JournalEntry *_jcur;

            std::stack<Lock *> _locks;

            void log_je(JournalEntry *je, void *src, int len) { }
            void release_locks() { }
            void finalize_commit() { }
            void abort() { }
            void rollback() { }
        public:
            TransactionImpl() { }
            ~TransactionImpl() { }

            void acquire_readlock(Lock *lptr) { /* throws on failure */ }
            void acquire_writelock(Lock *lptr) { /* throws on failure */ }

            // log data; user performs the writes
            void log(void *ptr, size_t len) { }

            // log old_val and write new_val
            template <typename T>
                void write(T *ptr, T new_val) { }

            // log dst and overwrite with src
            void write(void *dst, void *src, size_t len) { }

            // memset without logging
            void memset_nolog(void *ptr, uint8_t val, size_t len) { }

            // write new_val without logging
            template <typename T>
                void write_nolog(T *ptr, T new_val) { }

            // write without logging
            void write_nolog(void *dst, void *src, size_t len) { }


            void commit() { }

            // get current transaction
            static TransactionImpl *get_tx() { return NULL; }

            // flush a range and pcommit
            static void flush_range(void *ptr, size_t len) { }

            // roll-back the transaction 
            static bool recover_tx(TransactionHandle *) { return true; }

    };
};

