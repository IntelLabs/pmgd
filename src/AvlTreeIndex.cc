/**
 * @file   AvlTreeIndex.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stack>
#include "AvlTreeIndex.h"
#include "iterator.h"
#include "List.h"
#include "IndexString.h"
#include "GraphImpl.h"

using namespace PMGD;

namespace PMGD {
    template <typename K, typename V> class AvlTreeIndex<K,V>::Compare {
        K _val;
        bool _equal;
    public:
        Compare(const K val, bool equal) : _val(val), _equal(equal) {}

        // This is to be used only for making sure the values
        // are not equal. No implications on < or > should be made.
        bool equals(const K &val1) const
            { return (_val == val1) && _equal; }

        bool lessthan(const K &val1) const
            { return (_val < val1); }

        bool lessthanequal(const K &val1) const
            { return (_val < val1) || equals(val1); }

        bool greaterthan(const K &val1) const
            { return (_val > val1); }

        bool greaterthanequal(const K &val1) const
            { return (_val > val1) || equals(val1); }
    };

    template <typename K, typename V> class AvlTreeIndex<K,V>::Stack
        : public std::stack<TreeNode *>
    {
    public:
        void clear() { std::stack<TreeNode *>::c.clear(); }
    };
}

template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start(TreeNode *root,
                                   const Compare &cmin, const Compare &cmax,
                                   Stack &path,
                                   TransactionImpl *tx)
{
    if (root == NULL)
        return;

    // The main class was locked. So _tree should not change underneath it.
    // Since the parent is locked, its ok to do the null check above before
    // locking further.
    tx->acquire_lock(TransactionImpl::IndexLock, root, false);
    if (cmin.lessthan(root->key)) {
        if (cmax.greaterthanequal(root->key))
            path.push(root);
        find_start(root->left, cmin, cmax, path, tx);
    }
    else if (cmin.equals(root->key))
            path.push(root);
    else  // Need to look for min in the right subtree
        find_start(root->right, cmin, cmax, path, tx);
}

// We have already traversed to the min in the tree. So when
// we backtrack through the stack, the only check we need to
// make is that the value <(=) max.
template <typename K, typename V>
void AvlTreeIndex<K,V>::add_right_tree(TreeNode *root, const Compare &cmax,
                                       Stack &path,
                                       TransactionImpl *tx)
{
    if (root == NULL)
        return;
    tx->acquire_lock(TransactionImpl::IndexLock, root, false);
    if (cmax.greaterthanequal(root->key))
        path.push(root);
    add_right_tree(root->left, cmax, path, tx);
}

// Find first element when no min given
template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start_min(TreeNode *root, const Compare &cmax,
                                       Stack &path,
                                       TransactionImpl *tx)
{
    if (root == NULL)
        return;
    tx->acquire_lock(TransactionImpl::IndexLock, root, false);

    // min is the lowest element in the tree
    if (cmax.greaterthanequal(root->key))
        path.push(root);
    find_start_min(root->left, cmax, path, tx);
}

// Find first element when no max given
template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start_max(TreeNode *root, const Compare &cmin,
                                       Stack &path,
                                       TransactionImpl *tx)
{
    if (root == NULL)
        return;
    tx->acquire_lock(TransactionImpl::IndexLock, root, false);

    if (cmin.lessthan(root->key)) {
        path.push(root);
        find_start_max(root->left, cmin, path, tx);
    }
    else if (cmin.equals(root->key))
        path.push(root);
    else  // Need to look for min in the right subtree
        find_start_max(root->right, cmin, path, tx);
}

// Add all elements when no max limit given
template <typename K, typename V>
void AvlTreeIndex<K,V>::add_full_right_tree(TreeNode *root,
                                            Stack &path,
                                            TransactionImpl *tx)
{
    if (root == NULL)
        return;
    tx->acquire_lock(TransactionImpl::IndexLock, root, false);
    path.push(root);
    add_full_right_tree(root->left, path, tx);
}

// Find first element when no min/max given
template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start_all(TreeNode *root, Stack &path,
        TransactionImpl *tx)
{
    if (root == NULL)
        return;
    tx->acquire_lock(TransactionImpl::IndexLock, root, false);
    path.push(root);
    find_start_all(root->left, path, tx);
}

// Find min element of tree that is not equal to given key.
// Same function works for traversing the remaining tree too.
template <typename K, typename V>
void AvlTreeIndex<K,V>::add_nodes_neq(TreeNode *root, const K &neq,
                                      Stack &path,
                                      TransactionImpl *tx)
{
    if (root == NULL)
        return;
    tx->acquire_lock(TransactionImpl::IndexLock, root, false);
    if (!(neq == root->key))
        path.push(root);
    else {
        // If the root is equal to the key, don't forget its right,
        // which we can add to the path without checking againt neq
        // because of uniqueness.
        if (root->right != NULL) {
            tx->acquire_lock(TransactionImpl::IndexLock, root->right, false);
            path.push(root->right);
        }
    }
    add_nodes_neq(root->left, neq, path, tx);
}

template <typename K, typename V>
void AvlTreeIndex<K,V>::find_node_neq(TreeNode *root, const Compare &cur,
                                 const K &neq, Stack &path,
                                 TransactionImpl *tx)
{
    if (root == NULL)
        return;
    tx->acquire_lock(TransactionImpl::IndexLock, root, false);

    if (neq == root->key) { // Found the unwanted node.
        // Skip the root then
        if (cur.lessthan(root->key))
            find_start_max(root->left, cur, path, tx);
        else if (cur.equals(root->key))
            find_start_all(root->right, path, tx);
        else
            find_start_max(root->right, cur, path, tx);
    }
    else {
        path.push(root);
        if (cur.lessthan(root->key))
            find_node_neq(root->left, cur, neq, path, tx);
        else if (cur.greaterthan(root->key)) // equal to already taken care of
            find_node_neq(root->right, cur, neq, path, tx);
    }
}

// Support functions for reverse iterators
template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start_reverse(TreeNode *root,
                                   const Compare &cmin, const Compare &cmax,
                                   Stack &path,
                                   TransactionImpl *tx)
{
    if (root == NULL)
        return;
    tx->acquire_lock(TransactionImpl::IndexLock, root, false);
    if (cmax.greaterthan(root->key)) {
        if (cmin.lessthanequal(root->key))
            path.push(root);
        find_start_reverse(root->right, cmin, cmax, path, tx);
    }
    else if (cmax.equals(root->key))
            path.push(root);
    else  // Need to look for max in the left subtree as well.
        find_start_reverse(root->left, cmin, cmax, path, tx);
}

// Find first element when no max given
template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start_max_reverse(TreeNode *root, const Compare &cmin,
                                       Stack &path,
                                       TransactionImpl *tx)
{
    if (root == NULL)
        return;
    tx->acquire_lock(TransactionImpl::IndexLock, root, false);
    if (cmin.lessthanequal(root->key))
        path.push(root);
    find_start_max_reverse(root->right, cmin, path, tx);
}

// We have already traversed to the max in the tree. So when
// we backtrack through the stack, the only check we need to
// make is that the value >(=) min.
template <typename K, typename V>
void AvlTreeIndex<K,V>::add_left_tree(TreeNode *root, const Compare &cmin,
                                       Stack &path,
                                       TransactionImpl *tx)
{
    if (root == NULL)
        return;
    tx->acquire_lock(TransactionImpl::IndexLock, root, false);
    if (cmin.lessthanequal(root->key))
        path.push(root);
    add_left_tree(root->right, cmin, path, tx);
}

// Find first element when no min given
template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start_min_reverse(TreeNode *root, const Compare &cmax,
                                       Stack &path,
                                       TransactionImpl *tx)
{
    if (root == NULL)
        return;
    tx->acquire_lock(TransactionImpl::IndexLock, root, false);

    if (cmax.greaterthan(root->key)) {
        path.push(root);
        find_start_min_reverse(root->right, cmax, path, tx);
    }
    else if (cmax.equals(root->key))
        path.push(root);
    else  // Need to look for max in the left subtree
        find_start_min_reverse(root->left, cmax, path, tx);
}

// Find first element when no min/max given
template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start_all_reverse(TreeNode *root, Stack &path,
                                               TransactionImpl *tx)
{
    if (root == NULL)
        return;
    tx->acquire_lock(TransactionImpl::IndexLock, root, false);
    path.push(root);
    find_start_all_reverse(root->right, path, tx);
}

// Add all elements when no max limit given
template <typename K, typename V>
void AvlTreeIndex<K,V>::add_full_left_tree(TreeNode *root,
                                           Stack &path,
                                           TransactionImpl *tx)
{
    if (root == NULL)
        return;
    tx->acquire_lock(TransactionImpl::IndexLock, root, false);
    path.push(root);
    add_full_left_tree(root->right, path, tx);
}

// Find max element of tree that is not equal to given key.
// Same function works for traversing the remaining tree too.
template <typename K, typename V>
void AvlTreeIndex<K,V>::add_nodes_neq_reverse(TreeNode *root, const K &neq,
                                      Stack &path,
                                      TransactionImpl *tx)
{
    if (root == NULL)
        return;
    tx->acquire_lock(TransactionImpl::IndexLock, root, false);
    if (!(neq == root->key))
        path.push(root);
    else {
        // If the root is equal to the key, don't forget its left,
        // which we can add to the path without checking againt neq
        // because of uniqueness.
        if (root->left != NULL) {
            tx->acquire_lock(TransactionImpl::IndexLock, root->left, false);
            path.push(root->left);
        }
    }
    add_nodes_neq_reverse(root->right, neq, path, tx);
}

template <typename K, typename V>
void AvlTreeIndex<K,V>::find_node_neq_reverse(TreeNode *root, const Compare &cur,
                                     const K &neq, Stack &path,
                                     TransactionImpl *tx)
{
    if (root == NULL)
        return;
    tx->acquire_lock(TransactionImpl::IndexLock, root, false);

    if (neq == root->key) { // Found the unwanted node.
        // Skip the root then
        if (cur.greaterthan(root->key))
            find_start_min_reverse(root->right, cur, path, tx);
        else if (cur.equals(root->key))
            find_start_all_reverse(root->left, path, tx);
        else
            find_start_min_reverse(root->left, cur, path, tx);
    }
    else {
        path.push(root);
        if (cur.greaterthan(root->key))
            find_node_neq_reverse(root->right, cur, neq, path, tx);
        else if (cur.lessthan(root->key))
            find_node_neq_reverse(root->left, cur, neq, path, tx);
    }
}

namespace PMGD {

// The BASE_DECLS macro is used to avoid repeating the list of typedefs
// and using declarations in all seven derived classes.
#define BASE_DECLS \
        typedef typename Index_IteratorImplBase<K>::IndexValue IndexValue; \
        typedef typename Index_IteratorImplBase<K>::IndexNode IndexNode; \
        typedef typename Index_IteratorImplBase<K>::Stack Stack; \
        using Index_IteratorImplBase<K>::_tree; \
        using Index_IteratorImplBase<K>::_curr; \
        using Index_IteratorImplBase<K>::_path; \
        using Index_IteratorImplBase<K>::_list_it; \
        using Index_IteratorImplBase<K>::_vacant_flag; \
        using Index_IteratorImplBase<K>::finish_init; \
        using Index_IteratorImplBase<K>::_tx; \
        using Index_IteratorImplBase<K>::_index_type;

    // These iterator implementations are specific to instantiations
    // of AvlTreeIndex<K, V> with V = List<void *>.
    template <typename K>
    class Index_IteratorImplBase : public Index::Index_IteratorImplIntf {
    protected:
        typedef List<void *> IndexValue;
        typedef AvlTreeIndex<K, IndexValue> IndexNode;
        typedef typename IndexNode::Stack Stack;

        IndexNode *const _tree;
        typename IndexNode::TreeNode *_curr;
        Stack _path;
        ListTraverser<void *> _list_it;
        bool _vacant_flag = false;
        TransactionImpl *_tx;
        Graph::IndexType _index_type;

        void finish_init() {
            if (!_path.empty()) {
                _curr = _path.top();
                _path.pop();
                _list_it.set(&_curr->value);
            }
        }

        virtual void _next() = 0;
        virtual void find_node(const typename IndexNode::Compare &) = 0;

    public:
        Index_IteratorImplBase(IndexNode *tree)
            : _tree(tree), _curr(NULL), _list_it(NULL),
              _tx(TransactionImpl::get_tx())
        {
            if (_tx->is_read_write()) {
                _tx->iterator_callbacks().register_iterator(this,
                    [this](void *list_node) { remove_notify(list_node); },
                    [this](void *tree)
                    { rebalance_notify(static_cast<IndexNode *>(tree)); });
            }
        }

        ~Index_IteratorImplBase()
        {
            if (_tx->is_read_write())
                _tx->iterator_callbacks().unregister_iterator(this);
        }

        operator bool() const { return _vacant_flag || bool(_list_it); }

        bool next()
        {
            // If _vacant_flag is set, the iterator has already advanced
            // to the next object, so just clear _vacant_flag.
            if (EXPECT_FALSE(_vacant_flag)) {
                _vacant_flag = false;
                return operator bool();
            }

            if (_list_it.next())
                return true;

            // current list iterator empty; move to the next TreeNode
            _next();
            if (_path.empty())
                return false;

            _curr = _path.top();
            _path.pop();
            _list_it.set(&_curr->value);
            return true;
        }

        void set_index_type(Graph::IndexType index_type)
          { _index_type = index_type; }

        void *ref() const
        {
            // _vacant_flag indicates that the object referred to by the
            // iterator has been removed from the index.
            if (EXPECT_FALSE(_vacant_flag))
                throw PMGDException(VacantIterator);
            void *value = _list_it.ref();
            TransactionImpl::lock(_index_type, value, false);
            return value;
        }

        void remove_notify(void *list_node)
        {
            // If the object referred to by the iterator is being removed
            // from the index, move to the next object.
            if (_list_it.check(list_node)) {
                // Clear _vacant_flag to ensure that next actually advances
                // the iterator, and then set _vacant_flag to indicate that
                // the current object has been removed from the index.
                _vacant_flag = false;
                next();
                _vacant_flag = true;
            }
        }

        void rebalance_notify(IndexNode *tree)
        {
            if (tree == _tree && _curr != NULL && bool(_list_it)) {
                _path.clear();
                typename IndexNode::Compare cur(_curr->key, true);
                find_node(cur);
                assert(!_path.empty());
                _curr = _path.top();
                _path.pop();
            }
        }
    };

    template <typename K>
    class IndexEq_IteratorImpl : public Index_IteratorImplBase<K> {
        BASE_DECLS

    public:
        IndexEq_IteratorImpl(IndexNode *tree, const K &key)
            : Index_IteratorImplBase<K>(tree)
            { _list_it.set(tree->find(key)); }

        void _next()
            { /* Once we finish the original list, we're done. */  }

        void find_node(const typename IndexNode::Compare &)
            { /* not called because _curr is NULL */ }
    };

    // Handle gele, gelt, gtle, gtlt, le, lt.
    template <typename K>
    class IndexRange_IteratorImpl : public Index_IteratorImplBase<K> {
        BASE_DECLS
        typename IndexNode::Compare _cmax;

    public:
        IndexRange_IteratorImpl(IndexNode *tree,
                                    const K &min, const K &max,
                                    bool incl_min, bool incl_max)
            : Index_IteratorImplBase<K>(tree),
              _cmax(max, incl_max)
        {
            typename IndexNode::Compare cmin(min, incl_min);
            _tree->find_start(tree->_tree, cmin, _cmax, _path, _tx);
            finish_init();
        }

        // When a max is given but no min is specified, next is same as
        // that for gele kind of cases. So just add a constructor.
        IndexRange_IteratorImpl(IndexNode *tree, const K &max, bool incl_max)
            : Index_IteratorImplBase<K>(tree),
              _cmax(max, incl_max)
        {
            _tree->find_start_min(tree->_tree, _cmax, _path, _tx);
            finish_init();
        }

        void _next()
            { _tree->add_right_tree(_curr->right, _cmax, _path, _tx); }

        void find_node(const typename IndexNode::Compare &cur)
            { _tree->find_start(_tree->_tree, cur, _cmax, _path, _tx); }
    };

    // Handle ge, gt, dont_care.
    template <typename K>
    class IndexRangeNomax_IteratorImpl : public Index_IteratorImplBase<K> {
        BASE_DECLS

    public:
        IndexRangeNomax_IteratorImpl(IndexNode *tree,
                                    const K &min,
                                    bool incl_min)
            : Index_IteratorImplBase<K>(tree)
        {
            typename IndexNode::Compare cmin(min, incl_min);
            _tree->find_start_max(tree->_tree, cmin, _path, _tx);
            finish_init();
        }

        // The dont_care case where no min and max are given.
        IndexRangeNomax_IteratorImpl(IndexNode *tree)
            : Index_IteratorImplBase<K>(tree)
        {
            _tree->find_start_all(tree->_tree, _path, _tx);
            finish_init();
        }

        void _next()
            { _tree->add_full_right_tree(_curr->right, _path, _tx); }

        void find_node(const typename IndexNode::Compare &cur)
            { _tree->find_start_max(_tree->_tree, cur, _path, _tx); }
    };

    template <typename K>
    class IndexRangeNeq_IteratorImpl : public Index_IteratorImplBase<K> {
        BASE_DECLS
        K _neq;

    public:
        IndexRangeNeq_IteratorImpl(IndexNode *tree, const K &neq)
            : Index_IteratorImplBase<K>(tree),
              _neq(neq)
        {
            // Get to the minimum of the tree but make sure that is
            // not the key itself
            _tree->add_nodes_neq(tree->_tree, _neq, _path, _tx);
            finish_init();
        }

        void _next()
            { _tree->add_nodes_neq(_curr->right, _neq, _path, _tx); }

        void find_node(const typename IndexNode::Compare &cur)
            { _tree->find_node_neq(_tree->_tree, cur, _neq, _path, _tx); }
    };

    // Reverse iterators.
    // Handle gele, gelt, gtle, gtlt, gt, ge.
    template <typename K>
    class IndexRangeReverse_IteratorImpl : public Index_IteratorImplBase<K> {
        BASE_DECLS
        typename IndexNode::Compare _cmin;

    public:
        IndexRangeReverse_IteratorImpl(IndexNode *tree,
                                    const K &min, const K &max,
                                    bool incl_min, bool incl_max)
            : Index_IteratorImplBase<K>(tree),
              _cmin(min, incl_min)
        {
            typename IndexNode::Compare cmax(max, incl_max);
            _tree->find_start_reverse(tree->_tree, _cmin, cmax, _path, _tx);
            finish_init();
        }

        // When a min is given but no max is specified, next is same as
        // that for gele kind of cases. So just add a constructor.
        IndexRangeReverse_IteratorImpl(IndexNode *tree, const K &min, bool incl_min)
            : Index_IteratorImplBase<K>(tree),
              _cmin(min, incl_min)
        {
            _tree->find_start_max_reverse(tree->_tree, _cmin, _path, _tx);
            finish_init();
        }

        void _next()
            { _tree->add_left_tree(_curr->left, _cmin, _path, _tx); }

        void find_node(const typename IndexNode::Compare &cur)
            { _tree->find_start_reverse(_tree->_tree, _cmin, cur, _path, _tx); }
    };

    // Handle lt, le, dont_care
    template <typename K>
    class IndexRangeNomin_IteratorImpl : public Index_IteratorImplBase<K> {
        BASE_DECLS

    public:
        IndexRangeNomin_IteratorImpl(IndexNode *tree, const K &max, bool incl_max)
            : Index_IteratorImplBase<K>(tree)
        {
            typename IndexNode::Compare cmax(max, incl_max);
            _tree->find_start_min_reverse(tree->_tree, cmax, _path, _tx);
            finish_init();
        }

        // The dont_care case where no min and max are given.
        IndexRangeNomin_IteratorImpl(IndexNode *tree)
            : Index_IteratorImplBase<K>(tree)
        {
            _tree->find_start_all_reverse(tree->_tree, _path, _tx);
            finish_init();
        }

        void _next()
            { _tree->add_full_left_tree(_curr->left, _path, _tx); }

        void find_node(const typename IndexNode::Compare &cur)
            { _tree->find_start_min_reverse(_tree->_tree, cur, _path, _tx); }
    };

    template <typename K>
    class IndexRangeNeqReverse_IteratorImpl : public Index_IteratorImplBase<K> {
        BASE_DECLS
        K _neq;

    public:
        IndexRangeNeqReverse_IteratorImpl(IndexNode *tree, const K &neq)
            : Index_IteratorImplBase<K>(tree),
              _neq(neq)
        {
            // Get to the minimum of the tree but make sure that is
            // not the key itself
            _tree->add_nodes_neq_reverse(tree->_tree, _neq, _path, _tx);
            finish_init();
        }

        void _next()
            { _tree->add_nodes_neq_reverse(_curr->left, _neq, _path, _tx); }

        void find_node(const typename IndexNode::Compare &cur)
            { _tree->find_node_neq_reverse(_tree->_tree, cur, _neq, _path, _tx); }
    };
}

template <typename K, typename V>
Index::Index_IteratorImplIntf *AvlTreeIndex<K,V>::get_iterator(Graph::IndexType index_type, bool reverse)
{
    // We can read lock the main index class here.
    TransactionImpl *tx = TransactionImpl::get_tx();
    tx->acquire_lock(TransactionImpl::IndexLock, this, false);
    Index_IteratorImplBase<K> *impl = NULL;

    if (!reverse)
        impl = new IndexRangeNomax_IteratorImpl<K>(this);
    else
        impl =  new IndexRangeNomin_IteratorImpl<K>(this);

    impl->set_index_type(index_type);
    return impl;
}

template <typename K, typename V>
Index::Index_IteratorImplIntf *AvlTreeIndex<K,V>::get_iterator(Graph::IndexType index_type, const K &key,
                                                    PropertyPredicate::Op op, bool reverse)
{
    // We can read lock the main index class here.
    TransactionImpl *tx = TransactionImpl::get_tx();
    tx->acquire_lock(TransactionImpl::IndexLock, this, false);
    Index_IteratorImplBase<K> *impl = NULL;
    switch (op) {
        case PropertyPredicate::Eq:
            impl = new IndexEq_IteratorImpl<K>(this, key);
            break;
        case PropertyPredicate::Ne:
            if (!reverse)
                impl = new IndexRangeNeq_IteratorImpl<K>(this, key);
            else
                impl = new IndexRangeNeqReverse_IteratorImpl<K>(this, key);
            break;
        // < or <= some max. But start from min of tree.
        case PropertyPredicate::Lt:
            if (!reverse)
                impl = new IndexRange_IteratorImpl<K>(this, key, false);
            else
                impl = new IndexRangeNomin_IteratorImpl<K>(this, key, false);
            break;
        case PropertyPredicate::Le:
            if (!reverse)
                impl = new IndexRange_IteratorImpl<K>(this, key, true);
            else
                impl = new IndexRangeNomin_IteratorImpl<K>(this, key, true);
            break;
        // > or >= some min. But go till the max of tree.
        case PropertyPredicate::Gt:
            if (!reverse)
                impl = new IndexRangeNomax_IteratorImpl<K>(this, key, false);
            else
                impl = new IndexRangeReverse_IteratorImpl<K>(this, key, false);
            break;
        case PropertyPredicate::Ge:
            if (!reverse)
                impl = new IndexRangeNomax_IteratorImpl<K>(this, key, true);
            else
                impl = new IndexRangeReverse_IteratorImpl<K>(this, key, true);
            break;
        default: // Since Index already checks ops, this shouldn't happen.
            assert(0);
            impl = 0;
            break;
    }

    impl->set_index_type(index_type);
    return impl;
}

template <typename K, typename V>
Index::Index_IteratorImplIntf *AvlTreeIndex<K,V>::get_iterator(Graph::IndexType index_type, const K &min,
                                                               const K& max, PropertyPredicate::Op op,
                                                               bool reverse)
{
    bool incl_min = true, incl_max = true;

    if (op == PropertyPredicate::GtLt) {
        incl_min = false;
        incl_max = false;
    }
    else if (op == PropertyPredicate::GeLt)
        incl_max = false;
    else if (op == PropertyPredicate::GtLe)
        incl_min = false;

    TransactionImpl *tx = TransactionImpl::get_tx();
    tx->acquire_lock(TransactionImpl::IndexLock, this, false);
    Index_IteratorImplBase<K> *impl = NULL;

    if (!reverse)
        impl =  new IndexRange_IteratorImpl<K>(this, min, max, incl_min, incl_max);
    else
        impl =  new IndexRangeReverse_IteratorImpl<K>(this, min, max, incl_min, incl_max);

    impl->set_index_type(index_type);
    return impl;
}

template <typename K, typename V>
void AvlTreeIndex<K,V>::stats_recursive(TreeNode *root, Graph::IndexStats &stats)
{
    if (root == NULL)
        return;
    List<void *> node_list = root->value;
    size_t num_elems = node_list.num_elems();
    stats.total_elements   += num_elems;
    stats.total_size_bytes += this->treenode_size(root);
    stats.total_size_bytes += num_elems * node_list.list_type_size();
    stats_recursive(root->left,  stats);
    stats_recursive(root->right, stats);
}

template <typename K, typename V>
void AvlTreeIndex<K,V>::stats_health_recursive(TreeNode *root,
                                                Graph::IndexStats &stats,
                                                size_t &avg_elem_per_node)
{
    // Perfect health (100) means that all the nodes in the tree has
    // average or less number of elements.
    // If a node has more than average number of indexed elements, then
    // the node will decrease the health of the index proportionally with
    // the percentage of the elements this node has.
    // This is, if a node has 30% of all the indexed elements, and has
    // more than the average number of elements, then it will take 30 from the
    // health of the index, the new heatlh value being 70.
    // Standard deviation may be another metric,
    // but does not work well in the case of AvlTree
    // The corner cases of this method are when the tree
    // has 0 or 1 treenode, both cases marked as perfect health.

    if (root == NULL)
        return;
    List<void *> node_list = root->value;
    size_t node_elements = node_list.num_elems();

    if (node_elements > avg_elem_per_node) {
        stats.health_factor -= (100*node_elements) / stats.total_elements;
    }
    stats_health_recursive(root->left,  stats, avg_elem_per_node);
    stats_health_recursive(root->right, stats, avg_elem_per_node);
}

template <typename K, typename V>
void AvlTreeIndex<K,V>::index_stats_info(Graph::IndexStats &stats)
{
    // unique_entry_size can be variable in the case of IndexStrings.
    // We will consider sizeof(TreeNode) as unique_entry_size,
    // but the actual size will be added in total_size_bytes.

    stats.unique_entry_size     = sizeof(TreeNode);
    stats.total_unique_entries  = this->num_elems();
    stats.total_elements        = 0;
    stats.total_size_bytes      = sizeof(*this);
    stats.health_factor         = 100;

    if (stats.total_unique_entries == 0)
        return;

    stats_recursive(this->_tree, stats);

    size_t avg_elem_per_node = stats.total_elements / stats.total_unique_entries;
    stats_health_recursive(this->_tree, stats, avg_elem_per_node);
}

// Explicitly instantiate any types that might be required
template class AvlTreeIndex<long long, List<void *>>;
template class AvlTreeIndex<bool, List<void *>>;
template class AvlTreeIndex<double, List<void *>>;
template class AvlTreeIndex<Time, List<void *>>;
template class AvlTreeIndex<IndexString, List<void *>>;
