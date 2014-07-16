#pragma once
#include <stack>
#include "Index.h"
#include "AvlTree.h"
#include "TransactionImpl.h"

namespace Jarvis {
    template<typename K, typename V> class AvlTreeIndex
                            : public Index, public AvlTree<K,V>
    {
        // Helper functions for the iterators to function.
        class Compare;

        void find_start(typename AvlTree<K,V>::TreeNode *root,
                const K &min, const K &max,
                Compare &cmin,
                Compare &cmax,
                std::stack<typename AvlTree<K,V>::TreeNode *> &path);
        void add_right_tree(typename AvlTree<K,V>::TreeNode *root,
                const K &max, Compare &cmax,
                std::stack<typename AvlTree<K,V>::TreeNode *> &path);
        void find_start_min(typename AvlTree<K,V>::TreeNode *root,
                      const K &max,
                      Compare &cmax,
                      std::stack<typename AvlTree<K,V>::TreeNode *> &path);
        void find_start_max(typename AvlTree<K,V>::TreeNode *root,
                      const K &min,
                      Compare &cmin,
                      std::stack<typename AvlTree<K,V>::TreeNode *> &path);
        void add_full_right_tree(typename AvlTree<K,V>::TreeNode *root,
                      std::stack<typename AvlTree<K,V>::TreeNode *> &path);
        void find_start_all(typename AvlTree<K,V>::TreeNode *root,
                      std::stack<typename AvlTree<K,V>::TreeNode *> &path);
        void add_nodes_neq(typename AvlTree<K,V>::TreeNode *root,
                      const K &neq,
                      std::stack<typename AvlTree<K,V>::TreeNode *> &path);

        template <class D> friend class IndexRange_NodeIteratorImpl;
        template <class D> friend class IndexRangeNomax_NodeIteratorImpl;
        template <class D> friend class IndexRangeAll_NodeIteratorImpl;
        template <class D> friend class IndexRangeNeq_NodeIteratorImpl;
    public:
        // Initialize both and they do their own transaction flush
        AvlTreeIndex(PropertyType ptype) : Index(ptype), AvlTree<K,V>()
        {
            // This will flush for both the base classes too.
            TransactionImpl::flush_range(this, sizeof *this);
        }

        using AvlTree<K,V>::add;
        using AvlTree<K,V>::remove;

        NodeIterator get_nodes();
        NodeIterator get_nodes(const K &key, PropertyPredicate::op_t op);
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
