#include <stddef.h>
#include <assert.h>
#include "TransactionManager.h"
#include "TransactionImpl.h"
#include "exception.h"
#include "arch.h"

using namespace Jarvis;

TransactionManager::TransactionManager(
            uint64_t *tx_id,
            uint64_t transaction_table_addr, uint64_t transaction_table_size,
            uint64_t journal_addr, uint64_t journal_size,
            bool create, bool read_only)
    : _cur_tx_id(tx_id),
      _tx_table(reinterpret_cast<TransactionHdr *>(transaction_table_addr)),
      _journal_addr(reinterpret_cast<void *>(journal_addr)),
      _max_transactions(transaction_table_size / sizeof (TransactionHdr)),
      _extent_size(journal_size / _max_transactions),
      _max_extents(journal_size / _extent_size)
{
    // The current implementation uses exactly one extent per transaction.
    // _max_extents is always equal to _max_transactions.
    // If the implementation is changed to allow multiple extents for a
    // transaction, the determination of _extent_size should change.
    // However, the computation of _max_transactions and _max_extents
    // and the following requirement will not need to change.
    if (_max_extents < _max_transactions)
        throw Exception(invalid_config);

    if (create) {
        reset_table();
        *_cur_tx_id = 0;
    } else if (!read_only)
        recover();
    else
        check_clean();
}

void *TransactionManager::tx_jbegin(int index)
{
    // Each entry in the tx-table maps to a unique extent in the journal
    assert(index < _max_extents);
    return static_cast<uint8_t *>(_journal_addr) + (index * _extent_size);
}

void *TransactionManager::tx_jend(int index)
{
    return static_cast<uint8_t *>(_journal_addr) + ((index+1) * _extent_size);
}

void TransactionManager::reset_table(void)
{
    for (int i = 0; i < _max_transactions; i++)
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
    for (int i = 0; i < _max_transactions; i++) {
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
    for (int i = 0; i < _max_transactions; i++)
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

    TransactionId tx_id = atomic_inc(*_cur_tx_id) + 1;
    clflush(_cur_tx_id);

    for (int i = 0; i < _max_transactions; i++) {
        TransactionHdr *hdr = &_tx_table[i];
        if (hdr->tx_id == 0 && cmpxchg(hdr->tx_id, (TransactionId)0, tx_id)) {
            clflush(hdr);
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
        // Writing 0 to the transaction-id commits the transaction
        TransactionHdr *hdr = &_tx_table[handle.index];
        hdr->tx_id = 0;
        clflush(hdr);
        persistent_barrier(24);
    }
}
