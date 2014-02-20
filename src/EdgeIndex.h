#pragma once

#include <stddef.h>
#include "allocator.h"
#include "stringid.h"
#include "List.h"
#include "exception.h"
#include "node.h"
#include "edge.h"

namespace Jarvis {
    template <typename Type1, typename Type2> class KeyValuePair {
        Type1 elem1;
        Type2 elem2;

    public:
        KeyValuePair(const Type1& e1, const Type2& e2)
        {
            elem1 = e1;
            elem2 = e2;
        }

        bool operator== (const KeyValuePair& val2) const
        {
            return (elem1 == val2.elem1);
        }
        bool operator< (const KeyValuePair& val2) const
        {
            return (elem1 < val2.elem1);
        }

        void set(const Type1 &e1, const Type2 &e2)
        {
            elem1 = e1;
            elem2 = e2;
        }
        void set_key(const Type1 &e1) { elem1 = e1; }
        void set_value(const Type2 &e2) { elem2 = e2; }
        const Type1 &key() const { return elem1; }
        const Type2 &value() const { return elem2; }
    };

    // This class sits in PM. No DRAM image. So create a pointer 
    // and typecast
    class EdgeIndex {
    public:
        typedef KeyValuePair<Edge *, Node *> EdgeNodePair;
        typedef List<EdgeNodePair> EdgeList;
        typedef List<EdgeNodePair>::ListType EdgePosition;
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
            const EdgePosition *get_list_head() const { return _list.begin(); }
            size_t num_elems() { return _list.num_elems(); }
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
        // This will remove the element based on edge pointer value
        void remove(const StringID key, Edge* edge, Node* node, FixedAllocator& allocator);
    };
}
