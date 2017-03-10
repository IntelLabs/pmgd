/**
 * @file   transaction.cc
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

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <thread>
#include "transaction.h"
#include "TransactionImpl.h"
#include "TransactionManager.h"
#include "GraphImpl.h"
#include "arch.h"

using namespace PMGD;

// Transaction definitions
Transaction::Transaction(Graph &db, int options)
    : _impl(new TransactionImpl(db._impl, options))
{
}

Transaction::~Transaction()
{
    delete _impl;
}

void Transaction::commit()
{
    _impl->commit();
    delete _impl;
    _impl = NULL;
}


// TransactionImpl definitions

// 64B Journal entries
const static uint8_t JE_MAX_LEN = 48;

struct TransactionImpl::JournalEntry {
    TransactionId tx_id : 56;
    uint64_t len : 8;
    void *addr;
    uint8_t data[JE_MAX_LEN];
};

THREAD TransactionImpl *TransactionImpl::_per_thread_tx = NULL;

TransactionImpl::TransactionImpl(GraphImpl *db, int options)
    : _db(db),
      _tx_type(options),
      _committed(false),
      _locks { Locks(db->node_locks()), Locks(db->edge_locks()), Locks(db->index_locks()) }
{
    static_assert(sizeof (TransactionImpl::JournalEntry) == 64, "Journal entry size is not 64 bytes.");

    bool read_write = _tx_type & Transaction::ReadWrite;

    if (read_write) {
        db->check_read_write();
        db->msync_options(_msync_needed, _always_msync);
    }

    // nested dependent transactions not supported yet
    if (_per_thread_tx != NULL && read_write
            && !(_tx_type & Transaction::Independent))
        throw PMGDException(NotImplemented);

    _tx_handle = db->transaction_manager().alloc_transaction(!read_write,
                                                        _msync_needed, _pending_commits);

    _jcur = jbegin();
    _outer_tx = _per_thread_tx;
    _per_thread_tx = this; // Install per-thread TX

    _alloc_id = -1;
}

TransactionImpl::~TransactionImpl()
{
    if (_tx_type & Transaction::ReadWrite) {
        if (!_committed) {
            rollback(_tx_handle, _jcur, _msync_needed, _pending_commits);
            _abort_callback_list.do_callbacks(this);
        }
        _alloc_id = -1;
        _finalize_callback_list.do_callbacks(this);
    }

    for (unsigned i = 0; i < NUM_LOCK_REGIONS; ++i)
        _locks[i].unlock_all();

    // Free the journal after everything is done.
    if (_tx_type & Transaction::ReadWrite) {
        TransactionManager *tx_manager = &_db->transaction_manager();
        tx_manager->free_transaction(_tx_handle, _msync_needed, _pending_commits);
    }

    _per_thread_tx = _outer_tx;
}

void TransactionImpl::log_je(void *src_ptr, size_t len)
{
    _jcur->len = uint8_t(len);
    _jcur->addr = src_ptr;
    memcpy(&_jcur->data[0], src_ptr, len);
    memory_barrier();
    _jcur->tx_id = tx_id();
    TransactionManager::flush(_jcur, _msync_needed, _pending_commits);
    _jcur++;
}

void TransactionImpl::log(void *ptr, size_t len)
{
    assert(len > 0);
    size_t je_entries = (len + JE_MAX_LEN) / JE_MAX_LEN;

    if (_jcur + je_entries >= jend()) {
        if (_tx_type & Transaction::ReadWrite)
            throw PMGDException(OutOfJournalSpace);
        else
            throw PMGDException(ReadOnly);
    }

    for (unsigned i = 0; i < je_entries - 1; i++) {
        log_je(ptr, JE_MAX_LEN);
        ptr = static_cast<char *>(ptr) + JE_MAX_LEN;
    }

    log_je(ptr, len % JE_MAX_LEN);
    TransactionManager::commit(_always_msync, _pending_commits);
}

void TransactionImpl::finalize_commit()
{
    _commit_callback_list.do_callbacks(this);

    // Flush (and make durable) dirty in-place data pointed to by log entries
    for (JournalEntry *je = jbegin(); je < _jcur; je++)
        TransactionManager::flush(je->addr, _msync_needed, _pending_commits);
    TransactionManager::commit(_msync_needed, _pending_commits);
}


template<typename T>
static inline T align_low(T var, size_t sz)
{
    return (T)((uint64_t)var & ~(sz-1));
}

template<typename T>
static inline T align_high(T var, size_t sz)
{
    return (T)(((uint64_t)var + (sz-1)) & ~(sz-1));
}

void TransactionImpl::flush_range(void *ptr, size_t len,
                                bool msync_needed,
                                RangeSet &pending_commits)
{
    // adjust the size to flush
    len = align_high(len + ((size_t)ptr & (64-1)), 64);

    char *addr, *eptr;
    for (addr = (char *)ptr, eptr = (char *)ptr+len; addr < eptr; addr += 64)
        TransactionManager::flush(addr, msync_needed, pending_commits);
}

void TransactionImpl::flush_range(void *ptr, size_t len)
{
    flush_range(ptr, len, _msync_needed, _pending_commits);
}

// static function to allow recovery from TransactionManager.
// There are no real TransactionImpl objects at recovery time.
void TransactionImpl::recover_tx(const TransactionHandle &h, bool msync_needed,
                                 RangeSet &pending_commits)
{
    JournalEntry *jend = static_cast<JournalEntry *>(h.jend);
    rollback(h, jend, msync_needed, pending_commits);
}

void TransactionImpl::rollback(const TransactionHandle &h,
                               const JournalEntry *jend,
                               bool msync_needed,
                               RangeSet &pending_commits)
{
    JournalEntry *je;
    JournalEntry *jbegin = static_cast<JournalEntry *>(h.jbegin);

    // find the last valid journal entry
    for (je = jbegin; je < jend && je->tx_id == h.id; je++);

    // rollback in the reverse order
    while (je-- > jbegin) {
        memcpy(je->addr, &je->data[0], je->len);
        TransactionManager::flush(je->addr, msync_needed, pending_commits);
    }

    // Unless we have NoMsync, rollback can safely commit in case
    // of PM=MSYNC
    TransactionManager::commit(msync_needed, pending_commits);
}

TransactionImpl::LockState TransactionImpl::Locks::acquire_lock(const void *addr, bool write)
{
    uint64_t stripeid = mainlock.get_stripe_id(addr);
    typedef LockStatusMap::iterator LockMapIt;
    std::pair<LockMapIt, bool> pair =
        mylocks.insert(std::pair<uint64_t, uint8_t>(stripeid, LockNotFound));
    LockMapIt it = pair.first;
    if (!pair.second) {  // Insert didn't happen cause lock existed
        if (write && it->second != WriteLock) {  // what we had was a read lock
            mainlock.upgrade_lock(stripeid);
            it->second = WriteLock;
            return ReadLock;
        }
        return (LockState)it->second;
    }
    else {  // Lock not in system.
        if (write) {
            mainlock.write_lock(stripeid);
            it->second = WriteLock;
        }
        else {
            mainlock.read_lock(stripeid);
            it->second = ReadLock;
        }
        return LockNotFound;
    }
}

void TransactionImpl::Locks::unlock_all()
{
    for (auto it = mylocks.begin(); it != mylocks.end(); ++it) {
        if (it->second == WriteLock)
            mainlock.write_unlock(it->first);
        else if (it->second == ReadLock)
            mainlock.read_unlock(it->first);
    }
    mylocks.clear();
}

