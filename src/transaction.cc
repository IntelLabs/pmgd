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
      _commit_callback_list(NULL)
{
    static_assert(sizeof (TransactionImpl::JournalEntry) == 64, "Journal entry size is not 64 bytes.");

    bool read_write = _tx_type & Transaction::ReadWrite;

    if (read_write)
        db->check_read_write();

    // nested dependent transactions not supported yet
    if (_per_thread_tx != NULL && read_write
            && !(_tx_type & Transaction::Independent))
        throw Exception(NotImplemented);

    _tx_handle = db->transaction_manager().alloc_transaction(!read_write);

    _jcur = jbegin();
    _outer_tx = _per_thread_tx;
    _per_thread_tx = this; // Install per-thread TX
}

TransactionImpl::~TransactionImpl()
{
    if (_tx_type & Transaction::ReadWrite) {
        if (_committed) {
            finalize_commit();
        } else {
            rollback(_tx_handle, _jcur);
        }
        TransactionManager *tx_manager = &_db->transaction_manager();
        tx_manager->free_transaction(_tx_handle);
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
    clflush(_jcur);
    _jcur++;
}

void TransactionImpl::log(void *ptr, size_t len)
{
    assert(len > 0);
    size_t je_entries = (len + JE_MAX_LEN) / JE_MAX_LEN;

    if (_jcur + je_entries >= jend()) {
        if (_tx_type & Transaction::ReadWrite)
            throw Exception(OutOfJournalSpace);
        else
            throw Exception(ReadOnly);
    }

    for (unsigned i = 0; i < je_entries - 1; i++) {
        log_je(ptr, JE_MAX_LEN);
        ptr = static_cast<char *>(ptr) + JE_MAX_LEN;
    }

    log_je(ptr, len % JE_MAX_LEN);
    persistent_barrier(31);
}

void TransactionImpl::finalize_commit()
{
    call_commit_callbacks();

    // Flush (and make durable) dirty in-place data pointed to by log entries
    for (JournalEntry *je = jbegin(); je < _jcur; je++)
        clflush(je->addr);
    persistent_barrier(34);
}


struct TransactionImpl::CommitCallbackItem {
    CommitCallbackItem *next;
    void *obj;
    CommitCallback f;
    void *cookie;
};

void *&TransactionImpl::register_commit_callback(void *obj, CommitCallback f)
{
    for (CommitCallbackItem *l = _commit_callback_list; l != NULL; l = l->next)
        if (l->obj == obj)
            return l->cookie;

    CommitCallbackItem *tmp = new CommitCallbackItem{_commit_callback_list, obj, f, NULL};
    _commit_callback_list = tmp;
    return tmp->cookie;
}

void TransactionImpl::call_commit_callbacks()
{
    for (CommitCallbackItem *l = _commit_callback_list; l != NULL; l = l->next)
        (*l->f)(this, l->obj, l->cookie);
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
void TransactionImpl::recover_tx(const TransactionHandle &h)
{
    JournalEntry *jend = static_cast<JournalEntry *>(h.jend);
    rollback(h, jend);
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
