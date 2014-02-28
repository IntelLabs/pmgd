#pragma once
#include <stdint.h>
#include "allocator.h"

namespace Jarvis {
    template<typename T> class List;

    // Simple one way sorted list right now that disallows duplicates
    // TODO Remove sorting or make it optional
    // This class *lives* in PM. Therefore, no constructor, just an init
    // function. That also means it needs an allocator reference each time 
    // an element is added or removed. 
    // TODO Add transaction and commit support
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
        List() { _list = NULL; }
        // The variables above should just get mapped to the right area
        // and init will be called only the first time. 
        void init()
        {
            _list = NULL;
            _num_elems = 0;
        }
        // When doing a simple assignment
        List(const List& src) : _list(src._list), _num_elems(src._num_elems) {}

        T* add(const T &value, Allocator &allocator);
        void remove(const T &value, Allocator &allocator);
        T* find(const T &val);
        size_t num_elems() const { return _num_elems; }
    };

    template <typename T> T* List<T>::add(const T &value, Allocator &allocator)
    {
        // Special case because of no header node and 
        // sorting constraint
        if (_list == NULL || value < _list->value) {
            ListType *temp1 = (ListType *)allocator.alloc(sizeof *temp1);
            temp1->value = value;
            temp1->next = _list;
            // TODO log
            _list = temp1;
            _num_elems++;
            return &(_list->value);
        }
        ListType *temp = _list;
        while (temp != NULL) {
            if (temp->value == value) {
                // Handle duplicates by not adding anything
                return &(temp->value);
            }
            if (temp->next == NULL || value < temp->next->value)
                break;
            else
                temp = temp->next;
        }
        // Otherwise create a node and add
        // TODO log
        ListType *temp1 = (ListType *)allocator.alloc(sizeof *temp1);
        temp1->value = value;
        temp1->next = temp->next;
        temp->next = temp1;
        _num_elems++;
        return &(temp1->value);
    }

    template <typename T> void List<T>::remove(const T &value, Allocator &allocator)
    {
        ListType *temp = _list;
        ListType *prev = _list;
        if (value == _list->value) {
            // TODO log
            _list = _list->next;
            allocator.free(temp, sizeof *temp);
            _num_elems--;
            return;
        }
        temp = _list->next;
        while (temp != NULL) {
            if (value == temp->value) {
                // TODO log
                prev->next = temp->next;
                allocator.free(temp, sizeof *temp);
                _num_elems--;
                return;
            }
            temp = temp->next;
            prev = temp;
        }
    }

    // TODO not tested
    template <typename T> T* List<T>::find(const T &value)
    {
        ListType *temp = _list;
        while (temp != NULL) {
            if (temp->value == value)
                return &(temp->value);
            temp = temp->next;
        }
        return NULL;
    }
}
