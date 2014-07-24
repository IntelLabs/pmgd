#include <stddef.h>
#include <assert.h>
#include "TransactionManager.h"
#include "TransactionImpl.h"
#include "exception.h"
#include "arch.h"

using namespace Jarvis;

TransactionManager::TransactionManager(
            uint64_t transaction_table_addr, uint64_t transaction_table_size,
            uint64_t journal_addr, uint64_t journal_size,
            bool create, bool read_only)
{
    assert(transaction_table_size >= TRANSACTION_TABLE_SIZE);
    assert(journal_size >= JOURNAL_SIZE);

    _tx_table = reinterpret_cast<TransactionHdr *>(transaction_table_addr);
    _journal_addr = reinterpret_cast<void *>(journal_addr);

    if (create)
        reset_table();
    else if (!read_only)
        recover();
    else
        check_clean();

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

void TransactionManager::check_clean()
{
    // We can't open a graph read-only if it needs recovery.
    // Check that there are no pending transactions, which would
    // indicate a prior un-clean close.
    for (int i = 0; i < MAX_TRANSACTIONS; i++)
        if (_tx_table[i].tx_id != 0)
            throw Exception(read_only);
}

TransactionHandle TransactionManager::alloc_transaction(bool read_only)
{
    if (read_only) {
        // For a read-only transaction, don't allocate a transaction ID
        // or an extent, but set jbegin and jend to a valid address,
        // so that the overflow check in TransactionImpl::log will
        // catch attempts to modify the graph using this transaction.
        // Note that dummy may point to an extent that is in use by
        // another transaction, but it will never be used except in
        // a pointer comparison.
        void *dummy = tx_jbegin(0);
        return TransactionHandle(-1, -1, dummy, dummy);
    }

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
    // If handle.index is -1, this is a read-only transaction, and
    // nothing needs to be done.
    if (handle.index != -1) {
        TransactionHdr *hdr = &_tx_table[handle.index];
        hdr->tx_id = 0;
        clflush(hdr);
        persistent_barrier(24); // Is this required?
    }
}
