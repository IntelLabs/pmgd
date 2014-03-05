#include <stddef.h>
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
}


// TransactionImpl definitions

// 64B Journal entries
const static uint8_t JE_MAX_LEN = 51;
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

static const uint32_t LOCK_TIMEOUT = 7;

TransactionImpl::TransactionImpl(GraphImpl *db, int options)
    : _db(db), _committed(false),
      _tx_handle(db->transaction_manager().alloc_transaction())
{
    _jcur = jbegin();
    _per_thread_tx = this; // Install per-thread TX
}

TransactionImpl::~TransactionImpl()
{
    if (_committed == true) {
        finalize_commit();
        printf("TX(%d) committed !\n", tx_id());
    } else {
        rollback();
        printf("TX(%d) aborted\n", tx_id());
    }
    release_locks();

    TransactionManager *tx_manager = &_db->transaction_manager();
    tx_manager->free_transaction(_tx_handle);
    _per_thread_tx = NULL; // Un-install per-thread TX
    printf("Transaction ended\n");
}


inline TransactionId TransactionImpl::tx_id() { return _tx_handle.id; }

// TODO
void TransactionImpl::acquire_readlock(Lock *lptr)
{
#if 0
    if (lptr->acquire_readlock(LOCK_TIMEOUT) == false)
        this->abort();
    _locks.push(lptr);
#endif
}

// TODO
void TransactionImpl::acquire_writelock(Lock *lptr)
{
#if 0
    if (lptr->acquire_writelock(LOCK_TIMEOUT) == false)
        this->abort();
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

void TransactionImpl::log_je(JournalEntry *je, void *src_ptr, uint8_t len)
{
    je->len = len;
    je->addr = src_ptr;
    memcpy(&je->data[0], je->addr, je->len);
    memory_barrier();
    je->tx_id = tx_id();
    clflush(je->addr); pcommit();
}

// The last record in the journal is fixed as the COMMIT record.
void TransactionImpl::log(void *ptr, size_t len)
{
    size_t je_entries = (len + JE_MAX_LEN) / JE_MAX_LEN;

    if (_jcur + je_entries >= jend() - 1) {
        throw Exception(tx_small_journal); return;
    }

    // TODO: Acquire lock to support multiple threads in a transaction
    for (; _jcur < _jcur+je_entries-1; _jcur++) {
        log_je(_jcur, ptr, JE_MAX_LEN);
        ptr = static_cast<char *>(ptr) + _jcur->len;
    }

    log_je(_jcur, ptr, static_cast<uint8_t>(len % JE_MAX_LEN));
    _jcur++;
}

void TransactionImpl::finalize_commit()
{
    for (JournalEntry *je = jbegin(); je < _jcur; je++) clflush(je);
    pcommit();
}

void TransactionImpl::rollback()
{
    for (JournalEntry *je = jbegin(); je < _jcur; je++) {
        memcpy(je->addr, &je->data[0], je->len);
        clflush(je->addr);
    }
    pcommit();
}

void TransactionImpl::commit()
{
    _committed = true;
}

void TransactionImpl::abort()
{
    _committed = false;
    throw Exception(tx_aborted);
}

// flush a range and pcommit
void TransactionImpl::flush_range(void *ptr, size_t len)
{
}

// static function to allow recovery from TransactionManager.
// There are no real TransactionImpl objects at recovery time.
// The last record in the journal is fixed as the COMMIT record.
bool TransactionImpl::recover_tx(const TransactionHandle &h)
{
    uint32_t tx_id = h.id;
    JournalEntry *jend = static_cast<JournalEntry *>(h.jend);

    JournalEntry *jcommit = --jend;
    if (jcommit->tx_id == tx_id && jcommit->type == JE_COMMIT_MARKER)
        return true;

    // COMMIT record not found. Rollback !
    JournalEntry *jbegin = static_cast<JournalEntry *>(h.jbegin);

    for (JournalEntry *je = jbegin; je < jend; je++) {
        if (je->tx_id != tx_id)
            break;
        memcpy(je->addr, &je->data[0], je->len);
        clflush(je->addr);
    }
    pcommit();

    return true;
}
