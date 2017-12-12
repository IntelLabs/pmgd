/**
 * @file   ChunkList.h
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
#include <stdint.h>
#include <string.h>
#include <vector>
#include "Allocator.h"
#include "KeyValuePair.h"
#include "TransactionImpl.h"
#include "RangeSet.h"

namespace PMGD {
    // List of chunks. Size passed at creation time. The number of
    // objects of type T per chunk will obviously depend on the size
    // of this chunk - (current number of elements in the chunk + a
    // pointer to the next chunk)
    template <typename K, typename V, unsigned CHUNK_SIZE> class ChunkList {
        struct ChunkListType {
            ChunkListType *next; // Pointer to next chunk
            uint16_t occupants;  // Bitmask to indicate filled spots in a chunk
            uint8_t num_elems;   // This is in this chunk
            // Since we want to make an array for both K,V, use KVP here.
            KeyValuePair<K,V> data[]; // Array of key,values
        };
        static const unsigned MAX_PER_CHUNK =
            (CHUNK_SIZE - sizeof (ChunkListType)) / sizeof(KeyValuePair<K,V>);
        static_assert(MAX_PER_CHUNK > 0 && MAX_PER_CHUNK <= 16, "Incorrect chunk size");

        ChunkListType *_head;      // First chunk
        size_t _num_elems;         // This is total so far

        const ChunkListType* begin() const { return _head; }

        // The unit test that needs printing access etc.
        friend class ChunkListTest;

    public:
        // Constructor for temporary objects. No need to log.
        // Assumes that the caller takes care of flushing the
        // header value.
        ChunkList()
        {
            _head = NULL;
            _num_elems = 0;
        }

        // The variables above should just get mapped to the right area
        // and init will be called only the first time.
        // Called as part of node creation. Will be flushed when
        // new node is flushed.
        void init(bool msync_needed, RangeSet &pending_commits)
        {
            *this = ChunkList();
            TransactionImpl::flush_range(this, sizeof *this,
                                         msync_needed, pending_commits);
        }

        V* add(const K &key, Allocator &allocator);
        void remove(const K &key, Allocator &allocator);
        V* find(const K &val);

        // While the stats like calls have locks on them, if they are not used
        // during quiescent period, they could cause a lot of LockTimeouts.
        size_t num_elems() const
        {
            TransactionImpl *tx = TransactionImpl::get_tx();
            tx->acquire_lock(TransactionImpl::IndexLock, this, false);
            return _num_elems;
        }

        size_t chunk_size_bytes() const { return CHUNK_SIZE; }
        size_t total_chunks() const;
        size_t health() const;
        size_t chunk_list_size() const {
            return total_chunks() * CHUNK_SIZE + sizeof(*this);
        }

        std::vector<KeyValuePair<K,V> *> get_key_values();
    };

    template <typename K, typename V, unsigned CHUNK_SIZE>
    size_t ChunkList<K,V,CHUNK_SIZE>::total_chunks() const
    {
        size_t total_chunks = 0;
        TransactionImpl *tx = TransactionImpl::get_tx();
        tx->acquire_lock(TransactionImpl::IndexLock, this, false);

        ChunkListType *curr = _head;
        while (curr != NULL) {  // across chunks
            tx->acquire_lock(TransactionImpl::IndexLock, curr, false);
            curr = curr->next;
            ++total_chunks;
        }

        return total_chunks;
    }

    template <typename K, typename V, unsigned CHUNK_SIZE>
    size_t ChunkList<K,V,CHUNK_SIZE>::health() const
    {
        if (total_chunks() == 0)
            return 100;

        // Measures the occupations of the chunks
        return 100 * num_elems() / (total_chunks()*MAX_PER_CHUNK);
    }

    template <typename K, typename V, unsigned CHUNK_SIZE>
    std::vector<KeyValuePair<K,V> *> ChunkList<K,V,CHUNK_SIZE>::get_key_values()
    {
        std::vector<KeyValuePair<K,V> *> key_values;
        TransactionImpl *tx = TransactionImpl::get_tx();
        tx->acquire_lock(TransactionImpl::IndexLock, this, false);

        ChunkListType *curr = _head;
        while (curr != NULL) {  // across chunks
            tx->acquire_lock(TransactionImpl::IndexLock, curr, false);
            // Within a chunk
            int elems = 0, curr_slot = 0;
            uint16_t bit_pos = 1;
            while (elems < curr->num_elems) {
                if (curr->occupants & bit_pos) {
                    key_values.push_back(&curr->data[curr_slot]);
                    elems++;
                }
                bit_pos <<= 1;
                // Keep current with index position
                ++curr_slot;
            }
            curr = curr->next;
        }

        return key_values;
    }

    template <typename K, typename V, unsigned CHUNK_SIZE>
    V* ChunkList<K,V,CHUNK_SIZE>::add(const K &key, Allocator &allocator)
    {
        // Acquire a read lock before traversing the chunk list otherwise
        // someone might modify _head itself.
        TransactionImpl *tx = TransactionImpl::get_tx();
        tx->acquire_lock(TransactionImpl::IndexLock, this, false);

        // Need to check if the key exists before we can add
        // If it exists, return pointer to that element. The caller
        // will have to use the value part of key,value to enter the
        // new property id in whichever data structure value points to.
        // If key doesn't exist, see if there is space in chunk, else
        // allocate new chunk like in a linked list and place this element
        // at the zeroeth index.
        ChunkListType *prev = NULL, *curr = _head;
        ChunkListType *empty_chunk = NULL;
        unsigned empty_slot = 0;

        // This list is not sorted, so search till the end
        while (curr != NULL) {  // across chunks
            // Lock at the chunk level to avoid too fine grained locking.
            // Besides, if there is a change, we have to change the bitmap
            // that represents the entire chunk.
            tx->acquire_lock(TransactionImpl::IndexLock, curr, false);

            // Within a chunk
            int elems = 0, curr_slot = 0;
            uint16_t bit_pos = 1;
            while (elems < curr->num_elems) {
                // Check if there is a 1 at the bit position being tested.
                // If the value is non-zero, there is, else not
                if (curr->occupants & bit_pos) { // non-empty slot
                    // elems is following the indexing since it is incremented
                    // after this check.
                    if (key == curr->data[curr_slot].key())
                        return &(curr->data[curr_slot].value());
                    elems++;
                }

                // If no key found and there is room in the chunk, remember
                // the chunk number but still keep looking. This could be a
                // hole cause some other key that was deleted from here
                else if (!empty_chunk) {
                    empty_chunk = curr;
                    empty_slot = curr_slot;
                }
                bit_pos <<= 1;

                // Keep current with index position
                ++curr_slot;
            }

            // If there is still some room left towards the end, and we have
            // not assigned anything to empty_chunk, do that now
            if (elems < MAX_PER_CHUNK && !empty_chunk) {
                empty_chunk = curr;
                empty_slot = curr_slot;
            }

            // If not found in this chunk, move to next
            prev = curr;
            curr = curr->next;
        }

        // Will cause a change in num_elems.
        tx->acquire_lock(TransactionImpl::IndexLock, this, true);

        // key not found
        if (empty_chunk) {
            uint16_t mask = 1;
            tx->acquire_lock(TransactionImpl::IndexLock, empty_chunk, true);
            mask <<= empty_slot;
            tx->log_range(&empty_chunk->occupants, &empty_chunk->num_elems);
            ++empty_chunk->num_elems;
            empty_chunk->occupants |= mask;

            curr = empty_chunk;
        }
        else { // Need a new chunk
            curr = (ChunkListType *)allocator.alloc(CHUNK_SIZE);
            curr->num_elems = 1;
            curr->occupants |= 1;
            curr->next = NULL;
            tx->flush_range(curr, sizeof (ChunkListType));

            if (prev == NULL) // First ever chunk
                tx->write(&_head, curr);
            else {
                tx->acquire_lock(TransactionImpl::IndexLock, prev, true);
                tx->write(&(prev->next), curr);
            }

            empty_slot = 0;
        }

        tx->write(&_num_elems, _num_elems + 1);

        curr->data[empty_slot].set_key(key);
        // Do a placement new here to make sure value is initialized
        void *space = &(curr->data[empty_slot].value());
        V *value = new (space) V();
        tx->flush_range(&curr->data[empty_slot], sizeof(KeyValuePair<K,V>));

        return value;
    }

    template <typename K, typename V, unsigned CHUNK_SIZE>
    void ChunkList<K,V,CHUNK_SIZE>::remove(const K &key, Allocator &allocator)
    {
        TransactionImpl *tx = TransactionImpl::get_tx();

        // Acquire a read lock before traaversing the chunk list otherwise
        // someone might modify _head itself.
        tx->acquire_lock(TransactionImpl::IndexLock, this, false);
        ChunkListType *prev = NULL, *curr = _head;

        // This list is not sorted, so search till the end
        while (curr != NULL) {  // across chunks
            tx->acquire_lock(TransactionImpl::IndexLock, curr, false);

            // Within a chunk
            int elems = 0, curr_slot = 0;
            uint16_t bit_pos = 1;
            while (elems < curr->num_elems) {
                // Check if there is a 1 at the bit position being tested.
                // If the value is non-zero, there is, else not
                if (curr->occupants & bit_pos) { // non-empty slot
                    if (key == curr->data[curr_slot].key()) {
                        // This lock order is the same as in insert.
                        tx->acquire_lock(TransactionImpl::IndexLock, this, true);
                        tx->acquire_lock(TransactionImpl::IndexLock, curr, true);
                        tx->log_range(&curr->occupants, &curr->num_elems);
                        --curr->num_elems;
                        curr->occupants &= (~bit_pos);

                        tx->write(&_num_elems, _num_elems - 1);
                        if (curr->num_elems == 0) {
                            // Need to free this chunk
                            if (prev == NULL)
                                tx->write(&_head, curr->next);
                            else {
                                tx->acquire_lock(TransactionImpl::IndexLock, prev, true);
                                tx->write(&(prev->next), curr->next);
                            }
                            allocator.free(curr, CHUNK_SIZE);
                        } // No more elements in this chunk
                        return;
                    }
                    elems++;
                } // if for bit pos not empty
                bit_pos <<= 1;
                // Keep current with index position
                ++curr_slot;
            } //within a chunk

            // If not found in this chunk, move to next
            prev = curr;
            curr = curr->next;
        }
    }

    template <typename K, typename V, unsigned CHUNK_SIZE>
    V* ChunkList<K,V,CHUNK_SIZE>::find(const K &key)
    {
        TransactionImpl *tx = TransactionImpl::get_tx();
        tx->acquire_lock(TransactionImpl::IndexLock, this, false);
        ChunkListType *curr = _head;

        // This list is not sorted, so search till the end
        while (curr != NULL) {  // across chunks
            tx->acquire_lock(TransactionImpl::IndexLock, curr, false);
            // Within a chunk
            int elems = 0, curr_slot = 0;
            uint16_t bit_pos = 1;
            while (elems < curr->num_elems) {
                // Check if there is a 1 at the bit position being tested.
                // If the value is non-zero, there is, else not
                if (curr->occupants & bit_pos) { // non-empty slot
                    if (key == curr->data[curr_slot].key())
                        return &(curr->data[curr_slot].value());
                    elems++;
                }
                bit_pos <<= 1;
                // Keep current with index position
                ++curr_slot;
            }

            // If not found in this chunk, move to next
            curr = curr->next;
        }
        return NULL;
    }
}
