#include <stddef.h>
#include <assert.h>
#include "TransactionManager.h"
#include "TransactionImpl.h"
#include "exception.h"
#include "arch.h"

using namespace Jarvis;

TransactionManager::TransactionManager(
        uint64_t region_addr, uint64_t region_size, bool create)
{
    assert(region_size >= TRANSACTION_REGION_SIZE);

    _tx_table = reinterpret_cast<TransactionHdr *>(region_addr);
    _journal_addr = reinterpret_cast<void *>(region_addr + TRANSACTION_TABLE_SIZE);

    if (create)
        reset_table();
    else
        recover();

    _cur_tx_id = 0;
}

void *TransactionManager::tx_jbegin(int index)
{
    // TODO: check that index is less than MAX_TRANSACTIONS
    // Each entry in the tx-table maps to a unique extent in the journal
    return static_cast<uint8_t *>(_journal_addr) + (index * JOURNAL_EXTENT);
}

void *TransactionManager::tx_jend(int index)
{
    return static_cast<uint8_t *>(_journal_addr) + ((index+1) * JOURNAL_EXTENT);
}

void TransactionManager::reset_table(void)
{
    for (int i = 0; i < MAX_TRANSACTIONS; i++)
    {
        TransactionHdr *hdr = &_tx_table[i];
        hdr->tx_id = 0;
        hdr->jbegin = tx_jbegin(i);
        hdr->jend = tx_jend(i);
        clflush(hdr);
    }
}

void TransactionManager::recover(void) {
    // create a Transaction(tx) object for each in_use entry and
    // call tx.abort (with no locks)
    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        TransactionHdr *hdr = &_tx_table[i];
        if (hdr->tx_id != 0) {
            TransactionHandle handle(hdr->tx_id, i, hdr->jbegin, hdr->jend);
            TransactionImpl::recover_tx(handle);
        }
    }
    reset_table(); // If successful, reset the journal
}

TransactionHandle TransactionManager::alloc_transaction()
{
    uint32_t tx_id = atomic_inc(_cur_tx_id) + 1;

    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        TransactionHdr *hdr = &_tx_table[i];
        if (hdr->tx_id == 0 && cmpxchg(hdr->tx_id, 0u, tx_id)) {
            clflush(hdr);
            persistent_barrier(21); // Is this required?
            return TransactionHandle(tx_id, i, tx_jbegin(i), tx_jend(i));
        }
    }
    throw Exception(tx_alloc_failed);
}

void TransactionManager::free_transaction(const TransactionHandle &handle)
{
    TransactionHdr *hdr = &_tx_table[handle.index];
    hdr->tx_id = 0;
    clflush(hdr);
    persistent_barrier(24); // Is this required?
}
