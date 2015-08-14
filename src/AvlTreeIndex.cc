#include <stack>
#include "AvlTreeIndex.h"
#include "iterator.h"
#include "List.h"
#include "IndexString.h"

using namespace Jarvis;

namespace Jarvis {
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
                                   Stack &path)
{
    if (root == NULL)
        return;
    if (cmin.lessthan(root->key)) {
        if (cmax.greaterthanequal(root->key))
            path.push(root);
        find_start(root->left, cmin, cmax, path);
    }
    else if (cmin.equals(root->key))
            path.push(root);
    else  // Need to look for min in the right subtree
        find_start(root->right, cmin, cmax, path);
}

// We have already traversed to the min in the tree. So when
// we backtrack through the stack, the only check we need to
// make is that the value <(=) max.
template <typename K, typename V>
void AvlTreeIndex<K,V>::add_right_tree(TreeNode *root, const Compare &cmax,
                                       Stack &path)
{
    if (root == NULL)
        return;
    if (cmax.greaterthanequal(root->key))
        path.push(root);
    add_right_tree(root->left, cmax, path);
}

// Find first element when no min given
template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start_min(TreeNode *root, const Compare &cmax,
                                       Stack &path)
{
    if (root == NULL)
        return;
    // min is the lowest element in the tree
    if (cmax.greaterthanequal(root->key))
        path.push(root);
    find_start_min(root->left, cmax, path);
}

// Find first element when no max given
template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start_max(TreeNode *root, const Compare &cmin,
                                       Stack &path)
{
    if (root == NULL)
        return;

    if (cmin.lessthan(root->key)) {
        path.push(root);
        find_start_max(root->left, cmin, path);
    }
    else if (cmin.equals(root->key))
        path.push(root);
    else  // Need to look for min in the right subtree
        find_start_max(root->right, cmin, path);
}

// Add all elements when no max limit given
template <typename K, typename V>
void AvlTreeIndex<K,V>::add_full_right_tree(TreeNode *root,
                                            Stack &path)
{
    if (root == NULL)
        return;
    path.push(root);
    add_full_right_tree(root->left, path);
}

// Find first element when no min/max given
template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start_all(TreeNode *root, Stack &path)
{
    if (root == NULL)
        return;
    path.push(root);
    find_start_all(root->left, path);
}

// Find min element of tree that is not equal to given key.
// Same function works for traversing the remaining tree too.
template <typename K, typename V>
void AvlTreeIndex<K,V>::add_nodes_neq(TreeNode *root, const K &neq,
                                      Stack &path)
{
    if (root == NULL)
        return;
    if (!(neq == root->key))
        path.push(root);
    else {
        // If the root is equal to the key, don't forget its right,
        // which we can add to the path without checking againt neq
        // because of uniqueness.
        if (root->right != NULL)
            path.push(root->right);
    }
    add_nodes_neq(root->left, neq, path);
}

// Support functions for reverse iterators
template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start_reverse(TreeNode *root,
                                   const Compare &cmin, const Compare &cmax,
                                   Stack &path)
{
    if (root == NULL)
        return;
    if (cmax.greaterthan(root->key)) {
        if (cmin.lessthanequal(root->key))
            path.push(root);
        find_start_reverse(root->right, cmin, cmax, path);
    }
    else if (cmax.equals(root->key))
            path.push(root);
    else  // Need to look for max in the left subtree as well.
        find_start_reverse(root->left, cmin, cmax, path);
}

// Find first element when no max given
template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start_max_reverse(TreeNode *root, const Compare &cmin,
                                       Stack &path)
{
    if (root == NULL)
        return;
    if (cmin.lessthanequal(root->key))
        path.push(root);
    find_start_max_reverse(root->right, cmin, path);
}

// We have already traversed to the max in the tree. So when
// we backtrack through the stack, the only check we need to
// make is that the value >(=) min.
template <typename K, typename V>
void AvlTreeIndex<K,V>::add_left_tree(TreeNode *root, const Compare &cmin,
                                       Stack &path)
{
    if (root == NULL)
        return;
    if (cmin.lessthanequal(root->key))
        path.push(root);
    add_left_tree(root->right, cmin, path);
}

// Find first element when no min given
template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start_min_reverse(TreeNode *root, const Compare &cmax,
                                       Stack &path)
{
    if (root == NULL)
        return;

    if (cmax.greaterthan(root->key)) {
        path.push(root);
        find_start_min_reverse(root->right, cmax, path);
    }
    else if (cmax.equals(root->key))
        path.push(root);
    else  // Need to look for max in the left subtree
        find_start_min_reverse(root->left, cmax, path);
}

// Find first element when no min/max given
template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start_all_reverse(TreeNode *root, Stack &path)
{
    if (root == NULL)
        return;
    path.push(root);
    find_start_all_reverse(root->right, path);
}

// Add all elements when no max limit given
template <typename K, typename V>
void AvlTreeIndex<K,V>::add_full_left_tree(TreeNode *root,
                                           Stack &path)
{
    if (root == NULL)
        return;
    path.push(root);
    add_full_left_tree(root->right, path);
}

// Find max element of tree that is not equal to given key.
// Same function works for traversing the remaining tree too.
template <typename K, typename V>
void AvlTreeIndex<K,V>::add_nodes_neq_reverse(TreeNode *root, const K &neq,
                                      Stack &path)
{
    if (root == NULL)
        return;
    if (!(neq == root->key))
        path.push(root);
    else {
        // If the root is equal to the key, don't forget its left,
        // which we can add to the path without checking againt neq
        // because of uniqueness.
        if (root->left != NULL)
            path.push(root->left);
    }
    add_nodes_neq_reverse(root->right, neq, path);
}

namespace Jarvis {

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
        using Index_IteratorImplBase<K>::finish_init;

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

        void finish_init() {
            if (!_path.empty()) {
                _curr = _path.top();
                _path.pop();
                _list_it.set(&_curr->value);
            }
        }

        virtual void _next() = 0;

    public:
        Index_IteratorImplBase(IndexNode *tree)
            : _tree(tree), _curr(NULL), _list_it(NULL)
            { }

        void *ref() const { return _list_it.ref(); }
        operator bool() const { return bool(_list_it); }

        bool next()
        {
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
            _tree->find_start(tree->_tree, cmin, _cmax, _path);
            finish_init();
        }

        // When a max is given but no min is specified, next is same as
        // that for gele kind of cases. So just add a constructor.
        IndexRange_IteratorImpl(IndexNode *tree, const K &max, bool incl_max)
            : Index_IteratorImplBase<K>(tree),
              _cmax(max, incl_max)
        {
            _tree->find_start_min(tree->_tree, _cmax, _path);
            finish_init();
        }

        void _next()
            { _tree->add_right_tree(_curr->right, _cmax, _path); }
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
            _tree->find_start_max(tree->_tree, cmin, _path);
            finish_init();
        }

        // The dont_care case where no min and max are given.
        IndexRangeNomax_IteratorImpl(IndexNode *tree)
            : Index_IteratorImplBase<K>(tree)
        {
            _tree->find_start_all(tree->_tree, _path);
            finish_init();
        }

        void _next()
            { _tree->add_full_right_tree(_curr->right, _path); }
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
            _tree->add_nodes_neq(tree->_tree, _neq, _path);
            finish_init();
        }

        void _next()
            { _tree->add_nodes_neq(_curr->right, _neq, _path); }
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
            _tree->find_start_reverse(tree->_tree, _cmin, cmax, _path);
            finish_init();
        }

        // When a min is given but no max is specified, next is same as
        // that for gele kind of cases. So just add a constructor.
        IndexRangeReverse_IteratorImpl(IndexNode *tree, const K &min, bool incl_min)
            : Index_IteratorImplBase<K>(tree),
              _cmin(min, incl_min)
        {
            _tree->find_start_max_reverse(tree->_tree, _cmin, _path);
            finish_init();
        }

        void _next()
            { _tree->add_left_tree(_curr->left, _cmin, _path); }
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
            _tree->find_start_min_reverse(tree->_tree, cmax, _path);
            finish_init();
        }

        // The dont_care case where no min and max are given.
        IndexRangeNomin_IteratorImpl(IndexNode *tree)
            : Index_IteratorImplBase<K>(tree)
        {
            _tree->find_start_all_reverse(tree->_tree, _path);
            finish_init();
        }

        void _next()
            { _tree->add_full_left_tree(_curr->left, _path); }
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
            _tree->add_nodes_neq_reverse(tree->_tree, _neq, _path);
            finish_init();
        }

        void _next()
            { _tree->add_nodes_neq_reverse(_curr->left, _neq, _path); }
    };
}

template <typename K, typename V>
Index::Index_IteratorImplIntf *AvlTreeIndex<K,V>::get_iterator(bool reverse)
{
    if (!reverse)
        return new IndexRangeNomax_IteratorImpl<K>(this);
    else
        return new IndexRangeNomin_IteratorImpl<K>(this);
}

template <typename K, typename V>
Index::Index_IteratorImplIntf *AvlTreeIndex<K,V>::get_iterator(const K &key, PropertyPredicate::Op op, bool reverse)
{
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

    return impl;
}

template <typename K, typename V>
Index::Index_IteratorImplIntf *AvlTreeIndex<K,V>::get_iterator(const K &min, const K& max,
                                          PropertyPredicate::Op op,
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

    if (!reverse)
        return new IndexRange_IteratorImpl<K>(this, min, max, incl_min, incl_max);
    else
        return new IndexRangeReverse_IteratorImpl<K>(this, min, max, incl_min, incl_max);
}

// Explicitly instantiate any types that might be required
template class AvlTreeIndex<long long, List<void *>>;
template class AvlTreeIndex<bool, List<void *>>;
template class AvlTreeIndex<double, List<void *>>;
template class AvlTreeIndex<Time, List<void *>>;
template class AvlTreeIndex<IndexString, List<void *>>;
