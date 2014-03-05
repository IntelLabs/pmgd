#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stack>
#include "TransactionManager.h"

namespace Jarvis {
    class GraphImpl;
    class Lock;

    class TransactionImpl {
            struct JournalEntry;

            static thread_local TransactionImpl *_per_thread_tx;

            GraphImpl *_db;
            bool _committed;

            TransactionHandle _tx_handle;
            JournalEntry *_jcur;

            std::stack<Lock *> _locks;

            void log_je(void *src, size_t len);
            void release_locks();
            void finalize_commit();
            void rollback();
            static void rollback(const TransactionHandle &h,
                                 const JournalEntry *jend);

            TransactionId tx_id() { return _tx_handle.id; }
            JournalEntry *jbegin()
                { return static_cast<JournalEntry *>(_tx_handle.jbegin); }
            JournalEntry *jend()
                { return static_cast<JournalEntry *>(_tx_handle.jend); }

        public:
            TransactionImpl(GraphImpl *db, int options);
            ~TransactionImpl();

            void acquire_readlock(Lock *lptr);
            void acquire_writelock(Lock *lptr);

            // log data; user performs the writes
            void log(void *ptr, size_t len);

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


            void commit() { _committed = true; }

            // get current transaction
            static TransactionImpl *get_tx() { return _per_thread_tx; }

            // flush a range and pcommit
            static void flush_range(void *ptr, size_t len);

            // roll-back the transaction
            static void recover_tx(const TransactionHandle &);
    };
};
