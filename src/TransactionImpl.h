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
#include "RangeSet.h"
#include "lock.h"

namespace PMGD {
    class GraphImpl;

    class TransactionImpl {
        public:
            enum LockTarget { NodeLock = Graph::IndexType::NodeIndex,
                              EdgeLock = Graph::IndexType::EdgeIndex,
                              IndexLock,
                              NUM_LOCK_REGIONS };
            enum LockState { LockNotFound = 0, ReadLock = 1, WriteLock = 2 };

        // For the iterator callbacks used by AvlTreeIndex and others, we need a
        // per transaction way of storing it. So capture
        // it in a local class here and instantiate it in TX. Then the
        // register and unregister functions will be the only ones in TX.
        class IteratorCallbacks
        {
            CallbackList<void *, void *> _iterator_remove_list;
            CallbackList<void *, void *> _iterator_rebalance_list;
            CallbackList<void *, const PropertyRef &> _property_iterator_list;

        public:
            void register_iterator(void *key,
                                   std::function<void(void *)> remove_callback)
            {
                _iterator_remove_list.register_callback(key, remove_callback);
            }

            void register_iterator(void *key,
                                   std::function<void(void *)> remove_callback,
                                   std::function<void(void *)> rebalance_callback)
            {
                _iterator_remove_list.register_callback(key, remove_callback);
                _iterator_rebalance_list.register_callback(key, rebalance_callback);
            }

            void unregister_iterator(void *key)
            {
                _iterator_remove_list.unregister_callback(key);
                _iterator_rebalance_list.unregister_callback(key);
            }

            void iterator_remove_notify(void *list_node) const
                { _iterator_remove_list.do_callbacks(list_node); }
            void iterator_rebalance_notify(void *tree) const
                { _iterator_rebalance_list.do_callbacks(tree); }

            void register_property_iterator(void *key, std::function<void(const PropertyRef &)> f)
                { _property_iterator_list.register_callback(key, f); }
            void unregister_property_iterator(void *key)
                { _property_iterator_list.unregister_callback(key); }
            void property_iterator_notify(const PropertyRef &p) const
                { _property_iterator_list.do_callbacks(p); }

        };


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

            // For Msync support
            bool _msync_needed;    // false only when NoMsync used for msync cases.
            bool _always_msync;  // true only when AlwaysMsync used for msync cases.
            RangeSet _pending_commits;

            // Information for locks that a TX could acquire.
            std::array<Locks, NUM_LOCK_REGIONS> _locks;

            // Index manager has code to handle iterator changes within a
            // transaction. Instantiate that here.
            IteratorCallbacks _iter_callbacks;

            void log_je(void *src, size_t len);
            void finalize_commit();
            static void rollback(const TransactionHandle &h,
                                 const JournalEntry *jend,
                                 bool msync_needed,
                                 RangeSet &pending_commits);

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

            static void lock_node(const void *node, bool write)
                { get_tx()->acquire_lock(NodeLock, node, write); }
            static void lock_edge(const void *edge, bool write)
                { get_tx()->acquire_lock(EdgeLock, edge, write); }
            static void lock(Graph::IndexType type, const void *obj, bool write)
                { get_tx()->acquire_lock(LockTarget(type), obj, write); }

            IteratorCallbacks &iterator_callbacks()
              { return _iter_callbacks; }

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
                    // If an exception occurs in finalize_commit, the transaction
                    // is not committed, and will be rolled back.
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

            RangeSet *get_pending_commits() { return &_pending_commits; }

            // flush a range using clflushopt. Caller must call
            // commit to ensure the flushed data is durable.
            static void flush_range(void *ptr, size_t len, bool msync_needed, RangeSet &pc);

            // Another variation where TX is already present in the caller.
            void flush_range(void *ptr, size_t len);

            // roll-back the transaction
            static void recover_tx(const TransactionHandle &, bool, RangeSet &);
    };
};
