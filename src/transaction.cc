#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <thread>
#include "transaction.h"
#include "TransactionImpl.h"
#include "TransactionManager.h"
#include "GraphImpl.h"
#include "arch.h"

using namespace Jarvis;

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
const static uint8_t JE_COMMIT_MARKER = 0x77;

struct TransactionImpl::JournalEntry {
    TransactionId tx_id;
    union {
        uint8_t len;
        uint8_t type;
    };
    void *addr;
    uint8_t data[JE_MAX_LEN];
};

thread_local TransactionImpl *TransactionImpl::_per_thread_tx = NULL;
thread_local std::stack<TransactionImpl *> _tx_stack;

static const uint32_t LOCK_TIMEOUT = 7;

TransactionImpl::TransactionImpl(GraphImpl *db, int options)
    : _db(db), _tx_type(options), _committed(false),
      _tx_handle(db->transaction_manager().alloc_transaction())
{
    // nested transaction
    if (_per_thread_tx != NULL) {
        if (_tx_type & Transaction::Independent) {
            _tx_stack.push(_per_thread_tx);
            _per_thread_tx = NULL;
        } else {
            db->transaction_manager().free_transaction(_tx_handle);
            throw Exception(not_implemented);
        }
    }

    _jcur = jbegin();
    _per_thread_tx = this; // Install per-thread TX
}

TransactionImpl::~TransactionImpl()
{
    if (_committed) {
        finalize_commit();
    } else {
        rollback(_tx_handle, _jcur);
    }
    release_locks();

    TransactionManager *tx_manager = &_db->transaction_manager();
    tx_manager->free_transaction(_tx_handle);

    if (!_tx_stack.empty()) {
        assert(_tx_type & Transaction::Independent);
        _per_thread_tx = _tx_stack.top(); // nested transactions
        _tx_stack.pop();
    } else {
        _per_thread_tx = NULL; // un-install per-thread tx
    }
}


// TODO
void TransactionImpl::acquire_readlock(Lock *lptr)
{
#if 0
    if (!lptr->acquire_readlock(LOCK_TIMEOUT))
        throw Exception(e_deadlock);
    _locks.push(lptr);
#endif
}

// TODO
void TransactionImpl::acquire_writelock(Lock *lptr)
{
#if 0
    if (!lptr->acquire_writelock(LOCK_TIMEOUT))
        throw Exception(e_deadlock);
    _locks.push(lptr);
#endif
}

// TODO
void TransactionImpl::release_locks()
{
#if 0
    while (!_locks.empty()) {
        ret = ret && _locks.top()->release_lock();
        _locks.pop();
    }
#endif
}

void TransactionImpl::log_je(void *src_ptr, size_t len)
{
    _jcur->len = uint8_t(len);
    _jcur->addr = src_ptr;
    memcpy(&_jcur->data[0], src_ptr, len);
    memory_barrier();
    _jcur->tx_id = tx_id();
    clflush(_jcur);
    _jcur++;
}

// The last record in the journal is fixed as the COMMIT record.
void TransactionImpl::log(void *ptr, size_t len)
{
    assert(len > 0);
    size_t je_entries = (len + JE_MAX_LEN - 1) / JE_MAX_LEN;

    if (_jcur + je_entries >= jend() - 1)
        throw Exception(tx_small_journal);

    // TODO: Acquire lock to support multiple threads in a transaction
    for (unsigned i = 0; i < je_entries - 1; i++) {
        log_je(ptr, JE_MAX_LEN);
        ptr = static_cast<char *>(ptr) + JE_MAX_LEN;
    }

    log_je(ptr, len % JE_MAX_LEN);
    persistent_barrier(31);
}

void TransactionImpl::finalize_commit()
{
    // Flush (and make durable) dirty in-place data pointed to by log entries
    for (JournalEntry *je = jbegin(); je < _jcur; je++)
        clflush(je->addr);
    persistent_barrier(34);

    // Log the commit record
    JournalEntry *jcommit = jend() - 1;
    jcommit->type = JE_COMMIT_MARKER;
    jcommit->tx_id = tx_id();
    clflush(jcommit);
    persistent_barrier(37);
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

void TransactionImpl::flush_range(void *ptr, size_t len)
{
    // adjust the size to flush
    len = align_high(len + ((size_t)ptr & (64-1)), 64);

    char *addr, *eptr;
    for (addr = (char *)ptr, eptr = (char *)ptr+len; addr < eptr; addr += 64)
        clflush(addr);
}

// static function to allow recovery from TransactionManager.
// There are no real TransactionImpl objects at recovery time.
// The last record in the journal is fixed as the COMMIT record.
void TransactionImpl::recover_tx(const TransactionHandle &h)
{
    TransactionId tx_id = h.id;
    JournalEntry *jend = static_cast<JournalEntry *>(h.jend);
    JournalEntry *jcommit = jend - 1;

    if (!(jcommit->tx_id == tx_id && jcommit->type == JE_COMMIT_MARKER)) {
        // COMMIT record not found. Rollback !
        rollback(h, jcommit);
    }
}

void TransactionImpl::rollback(const TransactionHandle &h,
                               const JournalEntry *jend)
{
    JournalEntry *je;
    JournalEntry *jbegin = static_cast<JournalEntry *>(h.jbegin);

    // find the last valid journal entry
    for (je = jbegin; je < jend && je->tx_id == h.id; je++);

    // rollback in the reverse order
    while (je-- > jbegin) {
        memcpy(je->addr, &je->data[0], je->len);
        clflush(je->addr);
    }
    persistent_barrier(40);
}
