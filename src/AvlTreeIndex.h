#pragma once
#include <stack>
#include "Index.h"
#include "AvlTree.h"
#include "TransactionImpl.h"

namespace Jarvis {
    template<typename K, typename V> class AvlTreeIndex
                            : public Index, public AvlTree<K,V>
    {
        class Compare;

        void find_start(typename AvlTree<K,V>::TreeNode *root,
                const K &min, const K &max,
                Compare &cmin,
                Compare &cmax,
                std::stack<typename AvlTree<K,V>::TreeNode *> &path);
        void add_right_tree(typename AvlTree<K,V>::TreeNode *root,
                const K &max, Compare &cmax,
                std::stack<typename AvlTree<K,V>::TreeNode *> &path);

        template <class D> friend class IndexRange_NodeIteratorImpl;
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
        NodeIterator get_nodes(const K &min, const K &max, PropertyPredicate::op_t op);
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
