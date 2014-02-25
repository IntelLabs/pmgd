#pragma once

#include <stddef.h>
#include "allocator.h"
#include "stringid.h"
#include "List.h"
#include "exception.h"
#include "node.h"
#include "edge.h"

namespace Jarvis {
    template <typename KeyType, typename ValueType> class KeyValuePair {
        KeyType _key;
        ValueType _value;

    public:
        KeyValuePair(const KeyType& k, const ValueType& v)
        {
            _key = k;
            _value = v;
        }

        bool operator== (const KeyValuePair& val2) const
        {
            return (_key == val2._key);
        }
        bool operator< (const KeyValuePair& val2) const
        {
            return (_key < val2._key);
        }

        void set(const KeyType &k, const ValueType &v)
        {
            _key = k;
            _value = v;
        }
        void set_key(const KeyType &k) { _key = k; }
        void set_value(const ValueType &v) { _value = v; }
        const KeyType &key() const { return _key; }
        const ValueType &value() const { return _value; }
    };

    // This class sits in PM. No DRAM image. So create a pointer 
    // and typecast
    class EdgeIndex {
        class EdgeIndexType;
    public:
        typedef KeyValuePair<Edge *, Node *> EdgeNodePair;
        typedef List<EdgeNodePair> EdgeList;
        typedef List<EdgeNodePair>::ListType EdgePosition;
        typedef List<EdgeIndexType>::ListType KeyPosition;
    private:
        class EdgeIndexType {
            // Tag values
            StringID _key;
            // List of Edge, (src/dest) Node references
            EdgeList _list;

        public:
            // Use for temp objects
            // List should get a default constructor which is fine
            EdgeIndexType(StringID key): _key(key), _list() {}

            // Use when adding a new binary tree element
            // Since this sits in PM too, just give an init function
            void init(StringID key, EdgeNodePair &pair, FixedAllocator &allocator)
            {
                _key = key;
                _list.init();
                _list.add(pair, allocator);
            }

            EdgeIndexType& operator= (const EdgeIndexType &src)
            {
                _key = src._key;
                _list = src._list;
                return *this;
            }
            bool operator== (const EdgeIndexType& val2) const
            {
                return (_key == val2._key);
            }
            bool operator< (const EdgeIndexType& val2) const
            {
                return (_key < val2._key);
            }

            // Use when list exists
            void add(const EdgeNodePair &pair, FixedAllocator &allocator)
            {
                _list.add(pair, allocator);
            }
            void remove(const EdgeNodePair &pair, FixedAllocator &allocator)
            {
                _list.remove(pair, allocator);
            }
            size_t num_elems() { return _list.num_elems(); }

            // For iterators
            const EdgePosition *get_first() const { return _list.begin(); }
            const StringID &get_key() const { return _key; }
        };

        // Data structure for the keys that come in. Choosing a list
        // since there shouldn't be too many tags per node. Eventually
        // we can make this adaptive.
        // The second element is how the pairs will be organized.
        // Choosing a simple list for that for now
        List<EdgeIndexType> _key_list;

    public:
        // SInce each of the data structures track their own stuff
        // this doesn't need a header, just needs to make sure these 
        // data structures place their headers correctly
        void init() { _key_list.init(); }
        static EdgeIndex *create(FixedAllocator &allocator)
        {
            EdgeIndex *edge_table = (EdgeIndex *)allocator.alloc();
            edge_table->init();
            return edge_table;
        }

        // TODO Could split pair into two things and then this would work
        // for strings also. Just provide multiple add methods with different
        // number and kinds of parameters
        void add(const StringID key, Edge* edge, Node* node, FixedAllocator &allocator);
        // For the iterator, give it head of PairList for the key
        const EdgePosition *get_first(StringID key);
        // For the iterator, give it head of the key list
        const KeyPosition *get_first() { return _key_list.begin(); }
        size_t num_elems() { return _key_list.num_elems(); }
        // This will remove the element based on edge pointer value
        void remove(const StringID key, Edge* edge, Node* node, FixedAllocator& allocator);
    };
}
