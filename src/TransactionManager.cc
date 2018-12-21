/**
 * @file   TransactionManager.cc
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
#include "TransactionManager.h"
#include "TransactionImpl.h"
#include "RangeSet.h"
#include "exception.h"
#include "arch.h"

using namespace PMGD;

TransactionManager::TransactionManager(
            uint64_t transaction_table_addr, uint64_t transaction_table_size,
            uint64_t journal_addr, uint64_t journal_size,
            CommonParams &params)
    : _tx_table(reinterpret_cast<TransactionHdr *>(transaction_table_addr)),
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
        throw PMGDException(InvalidConfig);

    if (params.create) {
        reset_table(params.msync_needed, *params.pending_commits);
        _cur_tx_id = 0;
    }
    else
        recover(params.read_only, params.msync_needed, *params.pending_commits);
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

void TransactionManager::reset_table(bool msync_needed, RangeSet &pending_commits)
{
    for (int i = 0; i < _max_transactions; i++)
    {
        TransactionHdr *hdr = &_tx_table[i];
        hdr->tx_id = 0;
        hdr->jbegin = tx_jbegin(i);
        hdr->jend = tx_jend(i);
        flush(hdr, msync_needed, pending_commits);
    }
}

void TransactionManager::recover(bool read_only, bool msync_needed, RangeSet &pending_commits)
{
    // If there are any active transactions in the transaction table,
    // create a TransactionHandle and call recover_tx for each one.
    // At the same time, determine the highest transaction ID that
    // has been used.
    // We can't open a graph read-only if it needs recovery. If there
    // are active transactions and the graph is read-only, throw an
    // exception.
    TransactionId max_tx_id = 0;
    for (int i = 0; i < _max_transactions; i++) {
        TransactionHdr *hdr = &_tx_table[i];
        TransactionId tx_id = hdr->tx_id;

        if (tx_id & TransactionHdr::ACTIVE) {
            if (read_only)
                throw PMGDException(ReadOnly);

            tx_id &= ~TransactionHdr::ACTIVE;
            TransactionHandle handle(tx_id, i, hdr->jbegin, hdr->jend);
            TransactionImpl::recover_tx(handle, msync_needed, pending_commits);

            hdr->tx_id = tx_id;
            flush(hdr, msync_needed, pending_commits);
        }

        if (tx_id > max_tx_id)
            max_tx_id = tx_id;
    }
    _cur_tx_id = max_tx_id;
}

TransactionHandle TransactionManager::alloc_transaction(bool read_only,
                                                        bool msync_needed,
                                                        RangeSet &pending_commits)
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

    TransactionId tx_id = atomic_inc(_cur_tx_id) + 1;

    for (int i = 0; i < _max_transactions; i++) {
        TransactionHdr *hdr = &_tx_table[i];
        TransactionId prev_tx_id = hdr->tx_id;
        if ((prev_tx_id & TransactionHdr::ACTIVE) == 0
            && cmpxchg(hdr->tx_id, prev_tx_id, tx_id | TransactionHdr::ACTIVE))
        {
            flush(hdr, msync_needed, pending_commits);
            return TransactionHandle(tx_id, i, tx_jbegin(i), tx_jend(i));
        }
    }
    throw PMGDException(OutOfTransactions);
}

void TransactionManager::free_transaction(const TransactionHandle &handle,
                                          bool msync_needed,
                                          RangeSet &pending_commits)
{
    // If handle.index is -1, this is a read-only transaction, and
    // nothing needs to be done.
    if (handle.index != -1) {
        // Writing 0 to the transaction-id commits the transaction
        TransactionHdr *hdr = &_tx_table[handle.index];
        hdr->tx_id &= ~TransactionHdr::ACTIVE;
        flush(hdr, msync_needed, pending_commits);
        commit(msync_needed, pending_commits);
    }
}
