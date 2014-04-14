#pragma once
#include <stdint.h>
#include <string.h>
#include "allocator.h"
#include "KeyValuePair.h"

namespace Jarvis {
    template<typename K, typename V, unsigned CHUNK_SIZE> class ChunkList;

    // List of chunks. Size passed at creation time. The number of
    // objects of type T per chunk will obviously depend on the size
    // of this chunk - (current number of elements in the chunk + a
    // pointer to the next chunk)
    template <typename K, typename V, unsigned CHUNK_SIZE> class ChunkList {
        //struct __attribute((packed)) ChunkListType {
        struct ChunkListType {
            ChunkListType *next; // Pointer to next chunk
            uint16_t occupants;  // Bitmask to indicate filled spots in a chunk
            uint8_t num_elems;   // This is in this chunk
            // Since we want to make an array for both K,V, use KVP here.
            KeyValuePair<K,V> data[]; // Array of key,values
        };
        static const unsigned MAX_PER_CHUNK =
            (CHUNK_SIZE - sizeof (ChunkListType)) / sizeof(KeyValuePair<K,V>);

        ChunkListType *_head;      // First chunk
        size_t _num_elems;         // This is total so far

        const ChunkListType* begin() const { return _head; }

        friend class ChunkListTest;

    public:
        // Constructor for temporary objects. No need to log.
        ChunkList()
        { _head = NULL; _num_elems = 0; }

        // The variables above should just get mapped to the right area
        // and init will be called only the first time.
        // Called as part of node creation. Will be flushed when
        // new node is flushed.
        void init() { *this = ChunkList(); }

        V* add(const K &key, Allocator &allocator);
        void remove(const K &key, Allocator &allocator);
        V* find(const K &val);
        size_t num_elems() const { return _num_elems; }
    };

    template <typename K, typename V,unsigned CHUNK_SIZE>
    V* ChunkList<K,V,CHUNK_SIZE>::add(const K &key, Allocator &allocator)
    {
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
            // Within a chunk
            int elems = 0, curr_slot = 0;
            uint16_t bit_pos = 1;
            while (elems < curr->num_elems) {
                // Check if there is a 1 at the bit position being tested.
                // If the value is non-zero, there is, else not
                int elem_idx = curr->occupants & bit_pos;
                if (elem_idx != 0) { // non-empty slot
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

        // key not found
        ++_num_elems;
        if (empty_chunk) {
            uint16_t mask = 1;
            mask <<= empty_slot;
            empty_chunk->data[empty_slot].set_key(key);
            ++empty_chunk->num_elems;
            empty_chunk->occupants |= mask;
            return &(empty_chunk->data[empty_slot].value());
        }
        else { // Need a new chunk
            curr = (ChunkListType *)allocator.alloc(CHUNK_SIZE);
            curr->data[0].set_key(key);
            // Value portions will already be zeroed out
            curr->num_elems = 1;
            curr->occupants |= 1;
            curr->next = NULL;
            if (prev == NULL) { // First ever chunk
                _head = curr;
            }
            else {
                prev->next = curr;
            }
        }
        return &(curr->data[0].value());
    }

    template <typename K, typename V,unsigned CHUNK_SIZE>
    void ChunkList<K,V,CHUNK_SIZE>::remove(const K &key, Allocator &allocator)
    {
        ChunkListType *prev = NULL, *curr = _head;
        // This list is not sorted, so search till the end
        while (curr != NULL) {  // across chunks
            // Within a chunk
            int elems = 0, curr_slot = 0;
            uint16_t bit_pos = 1;
            while (elems < curr->num_elems) {
                // Check if there is a 1 at the bit position being tested.
                // If the value is non-zero, there is, else not
                int elem_idx = curr->occupants & bit_pos;
                if (elem_idx != 0) { // non-empty slot
                    if (key == curr->data[curr_slot].key()) {
                        --_num_elems;
                        --curr->num_elems;
                        curr->occupants &= (~bit_pos);
                        printf("Bitmask after removing key: 0x%x\n", curr->occupants);
                        if (curr->num_elems == 0) {
                            // Need to free this chunk
                            if (prev == NULL)
                                _head = curr->next;
                            else
                                prev->next = curr->next;
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

    template <typename K, typename V,unsigned CHUNK_SIZE>
    V* ChunkList<K,V,CHUNK_SIZE>::find(const K &key)
    {
        ChunkListType *curr = _head;
        // This list is not sorted, so search till the end
        while (curr != NULL) {  // across chunks
            // Within a chunk
            int elems = 0, curr_slot = 0;
            uint16_t bit_pos = 1;
            while (elems < curr->num_elems) {
                // Check if there is a 1 at the bit position being tested.
                // If the value is non-zero, there is, else not
                int elem_idx = curr->occupants & bit_pos;
                if (elem_idx != 0) { // non-empty slot
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
