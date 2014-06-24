#pragma once
#include "Index.h"
#include "AvlTree.h"
#include "TransactionImpl.h"

namespace Jarvis {
    template<typename K, typename V> class AvlTreeIndex
                            : public Index, public AvlTree<K,V>
    {
    public:
        // Initialize both and they do their own transaction flush
        AvlTreeIndex(PropertyType ptype) : Index(ptype), AvlTree<K,V>()
        {
            // This will flush for both the base classes too.
            TransactionImpl::flush_range(this, sizeof *this);
        }

        using AvlTree<K,V>::add;
        using AvlTree<K,V>::remove;

        // TODO: This could be a good location for an optimized tree
        // remove with the knowledge of V = list

        NodeIterator get_nodes(const K &key, const PropertyPredicate &pp);
    };

    // For the actual property value indices
    class Node;
    class IndexString;
    template <class T> class List;
    typedef AvlTreeIndex<long long, List<Node *>> LongValueIndex;
    typedef AvlTreeIndex<double, List<Node *>> FloatValueIndex;
    typedef AvlTreeIndex<bool, List<Node *>> BoolValueIndex;
    typedef AvlTreeIndex<IndexString, List<Node *>> StringValueIndex;
}
