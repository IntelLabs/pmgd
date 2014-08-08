#pragma once
#include <stack>
#include "Index.h"
#include "AvlTree.h"
#include "TransactionImpl.h"

namespace Jarvis {
    template<typename K, typename V> class AvlTreeIndex
                            : public Index, public AvlTree<K,V>
    {
        typedef typename AvlTree<K,V>::TreeNode TreeNode;

        // Helper functions for the iterators to function.
        class Compare;

        void find_start(TreeNode *root, const Compare &cmin, const Compare &cmax,
                        std::stack<TreeNode *> &path);
        void add_right_tree(TreeNode *root, const Compare &cmax,
                            std::stack<TreeNode *> &path);
        void find_start_min(TreeNode *root, const Compare &cmax,
                            std::stack<TreeNode *> &path);
        void find_start_max(TreeNode *root, const Compare &cmin,
                            std::stack<TreeNode *> &path);
        void add_full_right_tree(TreeNode *root, std::stack<TreeNode *> &path);
        void find_start_all(TreeNode *root, std::stack<TreeNode *> &path);
        void add_nodes_neq(TreeNode *root, const K &neq, std::stack<TreeNode *> &path);

        // For reverse iterators
        void find_start_reverse(TreeNode *root, const Compare &cmin, const Compare &cmax,
                        std::stack<TreeNode *> &path);
        void add_left_tree(TreeNode *root, const Compare &cmin,
                            std::stack<TreeNode *> &path);
        void find_start_max_reverse(TreeNode *root, const Compare &cmin,
                            std::stack<TreeNode *> &path);
        void find_start_min_reverse(TreeNode *root, const Compare &cmax,
                            std::stack<TreeNode *> &path);
        void find_start_all_reverse(TreeNode *root, std::stack<TreeNode *> &path);
        void add_full_left_tree(TreeNode *root, std::stack<TreeNode *> &path);
        void add_nodes_neq_reverse(TreeNode *root, const K &neq, std::stack<TreeNode *> &path);

        template <class D> friend class IndexRange_NodeIteratorImpl;
        template <class D> friend class IndexRangeNomax_NodeIteratorImpl;
        template <class D> friend class IndexRangeNeq_NodeIteratorImpl;
        template <class D> friend class IndexRangeReverse_NodeIteratorImpl;
        template <class D> friend class IndexRangeNomin_NodeIteratorImpl;
        template <class D> friend class IndexRangeNeqReverse_NodeIteratorImpl;
    public:
        // Initialize both and they do their own transaction flush
        AvlTreeIndex(PropertyType ptype) : Index(ptype), AvlTree<K,V>()
        {
            // This will flush for both the base classes too.
            TransactionImpl::flush_range(this, sizeof *this);
        }

        using AvlTree<K,V>::add;
        using AvlTree<K,V>::remove;

        NodeIterator get_nodes(bool reverse);
        NodeIterator get_nodes(const K &key, PropertyPredicate::op_t op, bool reverse);
        NodeIterator get_nodes(const K &min, const K &max, PropertyPredicate::op_t op,
                               bool reverse);
    };

    // For the actual property value indices
    class Node;
    class IndexString;
    template <class T> class List;
    typedef AvlTreeIndex<long long, List<Node *>> LongValueIndex;
    typedef AvlTreeIndex<double, List<Node *>> FloatValueIndex;
    typedef AvlTreeIndex<bool, List<Node *>> BoolValueIndex;
    typedef AvlTreeIndex<Time, List<Node *>> TimeValueIndex;
    typedef AvlTreeIndex<IndexString, List<Node *>> StringValueIndex;
}
