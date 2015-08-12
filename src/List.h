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
        friend class EdgeIndex;

        template <typename D> friend class ListTraverser;
        const ListType* begin() const { return _list; }

    public:
        // Constructor for temporary objects. No need to log.
        List() { _list = NULL; _num_elems = 0; }

        // The variables above should just get mapped to the right area
        // and init will be called only the first time.
        // Called as part of node creation. Will be flushed when
        // new node is flushed.
        void init() { *this = List(); }

        T* add(const T &value, Allocator &allocator);
        void remove(const T &value, Allocator &allocator);
        T* find(const T &val);
        size_t num_elems() const { return _num_elems; }
    };

    template <typename T> class ListTraverser {
        typename List<T>::ListType *_pos;

    public:
        void set(List<T> *l)
        {
            if (l)
                _pos = l->_list;
            else
                _pos = NULL;
        }
        ListTraverser(List<T> *l) { set(l); }
        const T &ref() const { return _pos->value; }
        operator bool() const { return _pos != NULL; }
        bool next()
        {
            _pos = _pos->next;
            return _pos != NULL;
        }
    };

    template <typename T> T* List<T>::add(const T &value, Allocator &allocator)
    {
        TransactionImpl *tx = TransactionImpl::get_tx();
        // Create a node and add
        ListType *new_node = (ListType *)allocator.alloc(sizeof *new_node);
        new_node->value = value;
        new_node->next = _list;
        // Since new_node is new allocation, just flush it without logging.
        TransactionImpl::flush_range(new_node, sizeof *new_node);

        // Since _list and _num_elems are contiguous, log() makes sense
        tx->log(this, sizeof *this);
        _list = new_node;
        _num_elems++;

        return &(new_node->value);
    }

    template <typename T> void List<T>::remove(const T &value, Allocator &allocator)
    {
        ListType *prev = NULL, *temp = _list;

        // Since we only implement the < operator on some of the T types,
        // using the reverse condition. The list is sorted, so stop after
        // reaching a larger element
        while (temp != NULL) {
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
        while (temp != NULL) {
            if (value == temp->value)
                return &(temp->value);
            temp = temp->next;
        }
        return NULL;
    }
}
