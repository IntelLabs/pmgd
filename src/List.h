#pragma once
#include <stdint.h>
#include "allocator.h"
#include "TransactionImpl.h"

namespace Jarvis {
    template<typename T> class List;

    // Simple one way sorted list right now that disallows duplicates
    // TODO Remove sorting or make it optional if needed
    // This class *lives* in PM. Therefore, no constructor, just an init
    // function. That also means it needs an allocator reference each time 
    // an element is added or removed. 
    template <typename T> class List {
        // < 32B even for a pair
        struct ListType {
            T value;
            ListType *next;
        };

        ListType *_list;
        size_t _num_elems;

        // For iterators
        friend class NodeEdgeIteratorImpl;
        friend class EdgeIndexType;
        friend class EdgeIndex;
        const ListType* begin() const { return _list; }

    public:
        // Constructor for temporary objects. No need to log.
        List() { _list = NULL; }
        // The variables above should just get mapped to the right area
        // and init will be called only the first time. 
        void init()
        {
            _list = NULL;
            _num_elems = 0;
        }

        T* add(const T &value, Allocator &allocator);
        void remove(const T &value, Allocator &allocator);
        T* find(const T &val);
        size_t num_elems() const { return _num_elems; }
    };

    template <typename T> T* List<T>::add(const T &value, Allocator &allocator)
    {
        ListType *prev = NULL, *next = _list;

        // Since we only implement the < operator on some of the T types,
        // using the reverse condition.
        while (next != NULL && !(value < next->value)) {
            if (value == next->value)
                return &(next->value);
            else {
                prev = next;
                next = next->next;
            }
        }
        TransactionImpl *tx = TransactionImpl::get_tx();
        // Create a node and add
        ListType *new_node = (ListType *)allocator.alloc(sizeof *new_node);
        new_node->value = value;
        new_node->next = next;
        // Since new_node is new allocation, just flush it without logging.
        TransactionImpl::flush_range(new_node, sizeof *new_node);

        if (prev == NULL) { // Insert at the start of list 
            // Since _list and _num_elems are contiguous, log() makes sense
            tx->log(this, sizeof *this);
            _list = new_node;
            _num_elems++;
        }
        else {
            // About to modify existing list elements, so log and update them
            tx->write(&(prev->next), new_node);
            tx->write(&_num_elems, _num_elems + 1);
        }
 
        return &(new_node->value);
    }

    template <typename T> void List<T>::remove(const T &value, Allocator &allocator)
    {
        ListType *prev = NULL, *temp = _list;
        
        // Since we only implement the < operator on some of the T types,
        // using the reverse condition. The list is sorted, so stop after
        // reaching a larger element
        while (temp != NULL && !(value < temp->value)) {
            if (value == temp->value) {
                TransactionImpl *tx = TransactionImpl::get_tx();
                if (prev == NULL) { // Changing _list
                    // Both members need to change
                    tx->log(this, sizeof *this);
                    _list = temp->next;
                    _num_elems--;
                }
                else {
                    tx->write(&(prev->next), temp->next);
                    tx->write(&_num_elems, _num_elems - 1);
                }
                allocator.free(temp, sizeof *temp);
                return;
            }
            prev = temp;
            temp = temp->next;
        }
    }

    template <typename T> T* List<T>::find(const T &value)
    {
        ListType *temp = _list;
        // Since we only implement the < operator on some of the T types,
        // using the reverse condition. The list is sorted, so stop after
        // reaching a larger element
        while (temp != NULL && !(value < temp->value)) {
            if (value == temp->value)
                return &(temp->value);
            temp = temp->next;
        }
        return NULL;
    }
}
