#include <stddef.h>
#include "TransactionManager.h"
#include "TransactionImpl.h"
#include "exception.h"

#define clflush(addr)
#define pcommit()
#define cmpxchg(ptr, oldval, newval) ({ *(ptr) = newval; oldval; })
#define atomic_inc(val) (++val)

using namespace Jarvis;

TransactionManager::TransactionManager(void *base_addr, bool create)
{
    _tx_table = static_cast<TransactionHdr *>(base_addr);
    _journal_addr = (uint8_t *)_tx_table + TRANSACTION_TABLE_SIZE;

    if (create)
        _reset_table();
    else
        _recover();

    _cur_tx_id = 0;
}

void *TransactionManager::_tx_jbegin(int index)
{
    // TODO: check that index is less than MAX_TRANSACTIONS
    // Each entry in the tx-table maps to a unique extent in the journal
    return static_cast<uint8_t *>(_journal_addr) + (index * JOURNAL_EXTENT);
}

void *TransactionManager::_tx_jend(int index)
{
    return static_cast<uint8_t *>(_journal_addr) + ((index+1) * JOURNAL_EXTENT);
}

void TransactionManager::_reset_table(void)
{
    for (int i = 0; i < MAX_TRANSACTIONS; i++)
    {
        TransactionHdr *hdr = &_tx_table[i];
        hdr->tx_id = 0;
        hdr->jbegin = _tx_jbegin(i);
        hdr->jend = _tx_jend(i);
        clflush(hdr);
    }
    pcommit();
}

void TransactionManager::_recover(void) {
    // create a Transaction(tx) object for each in_use entry and
    // call tx.abort (with no locks)
    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        TransactionHdr *hdr = &_tx_table[i];
        if (hdr->tx_id == 0) continue;

        TransactionHandle *handle = new TransactionHandle(
                hdr->tx_id, i, hdr->jbegin, hdr->jend);
        if (!handle)
            throw Exception(bad_alloc);

        if (TransactionImpl::recover_tx(handle) == false)
            throw Exception(tx_recovery_failed);
    }
    _reset_table(); // If successful, reset the journal
}

TransactionHandle *TransactionManager::alloc_transaction(void)
{
    uint32_t tx_id = atomic_inc(_cur_tx_id);

    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        TransactionHdr *hdr = &_tx_table[i];
        if (hdr->tx_id == 0 && cmpxchg(&hdr->tx_id, 0, tx_id) == 0) {
            clflush(hdr); pcommit();
            TransactionHandle *handle = new TransactionHandle(
                            tx_id, i, _tx_jbegin(i), _tx_jend(i));
            return handle;
        }
    }
    throw Exception(tx_alloc_failed);
    return NULL;
}

void TransactionManager::free_transaction(TransactionHandle *handle)
{
    if (handle) {
        TransactionHdr *hdr = &_tx_table[handle->get_index()];
        hdr->tx_id = 0;
        clflush(hdr); pcommit(); // no need of pcommit ?
    }
}

