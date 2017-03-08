/**
 * @file   TransactionImpl.h
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
#include <string.h>
#include <unordered_map>
#include <array>
#include "TransactionManager.h"
#include "exception.h"
#include "transaction.h"
#include "callback.h"
#include "compiler.h"
#include "lock.h"

namespace PMGD {
    class GraphImpl;

    class TransactionImpl {
        public:
            enum LockTarget { NodeLock = 0, EdgeLock = 1, IndexLock = 2, NUM_LOCK_REGIONS = 3};
            enum LockState { LockNotFound = 0, ReadLock = 1, WriteLock = 2 };

        private:
            typedef std::unordered_map<uint64_t, uint8_t> LockStatusMap;
            struct Locks {
                // Map of stripe id vs. read/write status
                LockStatusMap mylocks; // locks acquired on existing components.
                StripedLock &mainlock;          // Reference to the main lock from GraphImpl.

                // We could maintain status of new objects separately but
                // that might not be worth doing.
                // TODO need some statistics on how often we hit in map,
                // how long it takes and so on.
                Locks(StripedLock &locks) : mainlock(locks) {}

                // Return value indicates the state of given lock before
                // this operation.
                LockState acquire_lock(const void *addr, bool write);
                void unlock_all();
            };

            struct JournalEntry;

            static THREAD TransactionImpl *_per_thread_tx;

            GraphImpl *_db;
            int _tx_type;
            bool _msync_needed;    // false only when NoMsync used for msync cases.
            bool _always_msync;  // true only when AlwaysMsync used for msync cases.
            bool _committed;

            TransactionHandle _tx_handle;
            JournalEntry *_jcur;

            TransactionImpl *_outer_tx;

            // This has items such as address to free.
            CallbackList<void *, TransactionImpl *> _commit_callback_list;

            // Certain things like restoring DRAM states in some components
            // only needs to happen in case of abort. So create a callback for that.
            // Use these two only for DRAM related changes. Else we need to figure
            // out right flushing etc.
            CallbackList<void *, TransactionImpl *> _abort_callback_list;

            // Some unlocks like the ones for allocator need to be called regardless
            // of whether there was anything in the commit list. So have a finalize
            // list that is called for all ReadWrite transactions.
            CallbackList<void *, TransactionImpl *> _finalize_callback_list;

            int _alloc_id;

            // Information for locks that a TX could acquire.
            std::array<Locks, NUM_LOCK_REGIONS> _locks;

            void log_je(void *src, size_t len);
            void finalize_commit();
            static void rollback(const TransactionHandle &h,
                                 const JournalEntry *jend,
                                 bool msync_needed);

            TransactionId tx_id() const { return _tx_handle.id; }
            JournalEntry *jbegin()
                { return static_cast<JournalEntry *>(_tx_handle.jbegin); }
            JournalEntry *jend()
                { return static_cast<JournalEntry *>(_tx_handle.jend); }

        public:
            TransactionImpl(const TransactionImpl &) = delete;
            void operator=(const TransactionImpl &) = delete;
            TransactionImpl(GraphImpl *db, int options);
            ~TransactionImpl();

            GraphImpl *get_db() const { return _db; }

            bool is_read_write() const
                { return _tx_type & Transaction::ReadWrite; }

            void check_read_write()
            {
                if (!(_tx_type & Transaction::ReadWrite))
                    throw PMGDException(ReadOnly);
            }

            void register_commit_callback(void *key, std::function<void(TransactionImpl *)> f)
                { _commit_callback_list.register_callback(key, f); }

            std::function<void(TransactionImpl *)> *lookup_commit_callback(void *key)
                { return _commit_callback_list.lookup_callback(key); }

            void register_abort_callback(void *key, std::function<void(TransactionImpl *)> f)
                { _abort_callback_list.register_callback(key, f); }

            std::function<void(TransactionImpl *)> *lookup_abort_callback(void *key)
                { return _abort_callback_list.lookup_callback(key); }

            void register_finalize_callback(void *key, std::function<void(TransactionImpl *)> f)
                { _finalize_callback_list.register_callback(key, f); }

            std::function<void(TransactionImpl *)> *lookup_finalize_callback(void *key)
                { return _finalize_callback_list.lookup_callback(key); }

            // For concurrency
            void set_allocator(int alloc_id) { _alloc_id = alloc_id; }

            int get_allocator() { return _alloc_id; }

            // Add locks that this TX doesn't already own.
            LockState acquire_lock(LockTarget which, const void *addr, bool write = false)
                { return _locks[which].acquire_lock(addr, write); }

            // log data; user performs the writes
            void log(void *ptr, size_t len);

            // log data; base to base+end
            template <typename T>
            void log_range(void *base, T *end)
                { log(base, (char *)(end + 1) - (char *)base); }

            // log old_val and write new_val
            template <typename T>
                void write(T *ptr, T new_val)
            {
                log(ptr, sizeof(T));
                *ptr = new_val;
            }

            // log dst and overwrite with src
            void write(void *dst, void *src, size_t len)
            {
                log(src, len);
                memcpy(dst, src, len);
            }

            // memset without logging
            // TBD: implement when there is a need.
            void memset_nolog(void *ptr, uint8_t val, size_t len);

            // write new_val without logging
            template <typename T>
            void write_nolog(T *ptr, T new_val)
            {
                *ptr = new_val;
                flush_range(ptr, sizeof *ptr);
            }

            // write without logging
            void write_nolog(void *dst, void *src, size_t len)
            {
                memcpy(dst, src, len);
                flush_range(dst, len);
            }

            void commit()
            {
                if (_tx_type & Transaction::ReadWrite) {
                    // Finalize calls clean free list which could cause lock
                    // timeout failure or sometimes JournalSpace exception.
                    // So catch it and treat it as a case for
                    // rollback.
                    finalize_commit();
                }
                _committed = true;
            }

            // get current transaction
            static inline TransactionImpl *get_tx()
            {
                if (_per_thread_tx == NULL)
                    throw PMGDException(NoTransaction);
                return _per_thread_tx;
            }

            // flush a range using clflushopt. Caller must call
            // commit to ensure the flushed data is durable.
            static void flush_range(void *ptr, size_t len, bool msync_needed);

            // Another variation where TX is already present in the caller.
            void flush_range(void *ptr, size_t len);

            // roll-back the transaction
            static void recover_tx(const TransactionHandle &, bool);
    };
};
