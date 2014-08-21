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
}

template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start(TreeNode *root,
                                   const Compare &cmin, const Compare &cmax,
                                   std::stack<TreeNode *> &path)
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
                                       std::stack<TreeNode *> &path)
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
                                       std::stack<TreeNode *> &path)
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
                                       std::stack<TreeNode *> &path)
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
                                            std::stack<TreeNode *> &path)
{
    if (root == NULL)
        return;
    path.push(root);
    add_full_right_tree(root->left, path);
}

// Find first element when no min/max given
template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start_all(TreeNode *root, std::stack<TreeNode *> &path)
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
                                      std::stack<TreeNode *> &path)
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
                                   std::stack<TreeNode *> &path)
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
                                       std::stack<TreeNode *> &path)
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
                                       std::stack<TreeNode *> &path)
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
                                       std::stack<TreeNode *> &path)
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
void AvlTreeIndex<K,V>::find_start_all_reverse(TreeNode *root, std::stack<TreeNode *> &path)
{
    if (root == NULL)
        return;
    path.push(root);
    find_start_all_reverse(root->right, path);
}

// Add all elements when no max limit given
template <typename K, typename V>
void AvlTreeIndex<K,V>::add_full_left_tree(TreeNode *root,
                                           std::stack<TreeNode *> &path)
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
                                      std::stack<TreeNode *> &path)
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
    // TODO: Currently, this is very specific for the V param of
    // the tree. Need to make sure that data structure is flexible
    // and we can accommodate edges easily too.
    class Index_NodeIteratorImplBase : public NodeIteratorImplIntf {
    protected:
        ListTraverser<Node *> _list_it;
    public:
        Index_NodeIteratorImplBase(List<Node *> *l) : _list_it(l) { }

        Node &operator*() { return **_list_it; }
        Node *operator->() { return *_list_it; }
        Node &operator*() const { return **_list_it; }
        Node *operator->() const { return *_list_it; }
        operator bool() const { return _list_it; }
        virtual bool next() = 0;
    };

    template <typename K>
    class IndexEq_NodeIteratorImpl : public Index_NodeIteratorImplBase {
        typedef List<Node *> IndexValue;
        typedef AvlTreeIndex<K, IndexValue> IndexNode;
    public:
        IndexEq_NodeIteratorImpl(IndexNode *tree, const K &key)
            : Index_NodeIteratorImplBase(tree->find(key))
        { }
        bool next() { return _list_it.next(); }
    };

    // Handle gele, gelt, gtle, gtlt, le, lt.
    template <typename K>
    class IndexRange_NodeIteratorImpl : public Index_NodeIteratorImplBase {
        typedef List<Node *> IndexValue;
        typedef AvlTreeIndex<K, IndexValue> IndexNode;
        IndexNode *_tree;
        typename IndexNode::TreeNode *_curr;
        typename IndexNode::Compare _cmax;
        std::stack<typename IndexNode::TreeNode *> _path;

    public:
        IndexRange_NodeIteratorImpl(IndexNode *tree,
                                    const K &min, const K &max,
                                    bool incl_min, bool incl_max)
            : Index_NodeIteratorImplBase(NULL),
              _tree(tree), _curr(NULL),
              _cmax(max, incl_max)
        {
            typename IndexNode::Compare cmin(min, incl_min);
            _tree->find_start(tree->_tree, cmin, _cmax, _path);
            if (!_path.empty()) {
                _curr = _path.top();
                _path.pop();
            }
            if (_curr)
                _list_it.set(&_curr->value);
        }

        // When a max is given but no min is specified, next is same as
        // that for gele kind of cases. So just add a constructor.
        IndexRange_NodeIteratorImpl(IndexNode *tree, const K &max, bool incl_max)
            : Index_NodeIteratorImplBase(NULL),
              _tree(tree), _curr(NULL),
              _cmax(max, incl_max)
        {
            _tree->find_start_min(tree->_tree, _cmax, _path);
            if (!_path.empty()) {
                _curr = _path.top();
                _path.pop();
            }
            if (_curr)
                _list_it.set(&_curr->value);
        }

        bool next() {
            if(!_list_it.next()) { // current list iterator empty
                typename IndexNode::TreeNode *temp = _curr->right;
                _curr = NULL;
                _tree->add_right_tree(temp, _cmax, _path);
                if (!_path.empty()) {
                    _curr = _path.top();
                    _path.pop();
                }
                if (!_curr) { // Check if something returned from stack
                    _list_it.set(NULL);
                    return false;
                }
                _list_it.set(&_curr->value);
            }
            // Else the list iterator has already done a next
            return true;
        }
    };

    // Handle ge, gt, dont_care.
    template <typename K>
    class IndexRangeNomax_NodeIteratorImpl : public Index_NodeIteratorImplBase {
        typedef List<Node *> IndexValue;
        typedef AvlTreeIndex<K, IndexValue> IndexNode;
        IndexNode *_tree;
        typename IndexNode::TreeNode *_curr;
        std::stack<typename IndexNode::TreeNode *> _path;

    public:
        IndexRangeNomax_NodeIteratorImpl(IndexNode *tree,
                                    const K &min,
                                    bool incl_min)
            : Index_NodeIteratorImplBase(NULL),
              _tree(tree), _curr(NULL)
        {
            typename IndexNode::Compare cmin(min, incl_min);
            _tree->find_start_max(tree->_tree, cmin, _path);
            if (!_path.empty()) {
                _curr = _path.top();
                _path.pop();
            }
            if (_curr)
                _list_it.set(&_curr->value);
        }

        // The dont_care case where no min and max are given.
        IndexRangeNomax_NodeIteratorImpl(IndexNode *tree)
            : Index_NodeIteratorImplBase(NULL),
              _tree(tree), _curr(NULL)
        {
            _tree->find_start_all(tree->_tree, _path);
            if (!_path.empty()) {
                _curr = _path.top();
                _path.pop();
            }
            if (_curr)
                _list_it.set(&_curr->value);
        }

        bool next() {
            if(!_list_it.next()) { // current list iterator empty
                typename IndexNode::TreeNode *temp = _curr->right;
                _curr = NULL;
                _tree->add_full_right_tree(temp, _path);
                if (!_path.empty()) {
                    _curr = _path.top();
                    _path.pop();
                }
                if (!_curr) { // Check if something returned from stack
                    _list_it.set(NULL);
                    return false;
                }
                _list_it.set(&_curr->value);
            }
            // Else the list iterator has already done a next
            return true;
        }
    };

    template <typename K>
    class IndexRangeNeq_NodeIteratorImpl : public Index_NodeIteratorImplBase {
        typedef List<Node *> IndexValue;
        typedef AvlTreeIndex<K, IndexValue> IndexNode;
        IndexNode *_tree;
        K _neq;
        typename IndexNode::TreeNode *_curr;
        std::stack<typename IndexNode::TreeNode *> _path;

    public:
        IndexRangeNeq_NodeIteratorImpl(IndexNode *tree, const K &neq)
            : Index_NodeIteratorImplBase(NULL),
              _tree(tree), _neq(neq),
              _curr(NULL)
        {
            // Get to the minimum of the tree but make sure that is
            // not the key itself
            _tree->add_nodes_neq(tree->_tree, _neq, _path);
            if (!_path.empty()) {
                _curr = _path.top();
                _path.pop();
            }
            if (_curr)
                _list_it.set(&_curr->value);
        }

        bool next() {
            if(!_list_it.next()) { // current list iterator empty
                typename IndexNode::TreeNode *temp = _curr->right;
                _curr = NULL;
                _tree->add_nodes_neq(temp, _neq, _path);
                if (!_path.empty()) {
                    _curr = _path.top();
                    _path.pop();
                }
                if (!_curr) { // Check if something returned from stack
                    _list_it.set(NULL);
                    return false;
                }
                _list_it.set(&_curr->value);
            }
            // Else the list iterator has already done a next
            return true;
        }
    };

    // Reverse iterators.
    // Handle gele, gelt, gtle, gtlt, gt, ge.
    template <typename K>
    class IndexRangeReverse_NodeIteratorImpl : public Index_NodeIteratorImplBase {
        typedef List<Node *> IndexValue;
        typedef AvlTreeIndex<K, IndexValue> IndexNode;
        IndexNode *_tree;
        typename IndexNode::TreeNode *_curr;
        typename IndexNode::Compare _cmin;
        std::stack<typename IndexNode::TreeNode *> _path;

    public:
        IndexRangeReverse_NodeIteratorImpl(IndexNode *tree,
                                    const K &min, const K &max,
                                    bool incl_min, bool incl_max)
            : Index_NodeIteratorImplBase(NULL),
              _tree(tree), _curr(NULL), _cmin(min, incl_min)
        {
            typename IndexNode::Compare cmax(max, incl_max);
            _tree->find_start_reverse(tree->_tree, _cmin, cmax, _path);
            if (!_path.empty()) {
                _curr = _path.top();
                _path.pop();
            }
            if (_curr)
                _list_it.set(&_curr->value);
        }

        // When a min is given but no max is specified, next is same as
        // that for gele kind of cases. So just add a constructor.
        IndexRangeReverse_NodeIteratorImpl(IndexNode *tree, const K &min, bool incl_min)
            : Index_NodeIteratorImplBase(NULL),
              _tree(tree), _curr(NULL), _cmin(min, incl_min)
        {
            _tree->find_start_max_reverse(tree->_tree, _cmin, _path);
            if (!_path.empty()) {
                _curr = _path.top();
                _path.pop();
            }
            if (_curr)
                _list_it.set(&_curr->value);
        }

        bool next() {
            if(!_list_it.next()) { // current list iterator empty
                typename IndexNode::TreeNode *temp = _curr->left;
                _curr = NULL;
                _tree->add_left_tree(temp, _cmin, _path);
                if (!_path.empty()) {
                    _curr = _path.top();
                    _path.pop();
                }
                if (!_curr) { // Check if something returned from stack
                    _list_it.set(NULL);
                    return false;
                }
                _list_it.set(&_curr->value);
            }
            // Else the list iterator has already done a next
            return true;
        }
    };

    // Handle lt, le, dont_care
    template <typename K>
    class IndexRangeNomin_NodeIteratorImpl : public Index_NodeIteratorImplBase {
        typedef List<Node *> IndexValue;
        typedef AvlTreeIndex<K, IndexValue> IndexNode;
        IndexNode *_tree;
        typename IndexNode::TreeNode *_curr;
        std::stack<typename IndexNode::TreeNode *> _path;

    public:
        IndexRangeNomin_NodeIteratorImpl(IndexNode *tree, const K &max, bool incl_max)
            : Index_NodeIteratorImplBase(NULL), _tree(tree), _curr(NULL)
        {
            typename IndexNode::Compare cmax(max, incl_max);
            _tree->find_start_min_reverse(tree->_tree, cmax, _path);
            if (!_path.empty()) {
                _curr = _path.top();
                _path.pop();
            }
            if (_curr)
                _list_it.set(&_curr->value);
        }

        // The dont_care case where no min and max are given.
        IndexRangeNomin_NodeIteratorImpl(IndexNode *tree)
            : Index_NodeIteratorImplBase(NULL), _tree(tree), _curr(NULL)
        {
            _tree->find_start_all_reverse(tree->_tree, _path);
            if (!_path.empty()) {
                _curr = _path.top();
                _path.pop();
            }
            if (_curr)
                _list_it.set(&_curr->value);
        }

        bool next() {
            if(!_list_it.next()) { // current list iterator empty
                typename IndexNode::TreeNode *temp = _curr->left;
                _curr = NULL;
                _tree->add_full_left_tree(temp, _path);
                if (!_path.empty()) {
                    _curr = _path.top();
                    _path.pop();
                }
                if (!_curr) { // Check if something returned from stack
                    _list_it.set(NULL);
                    return false;
                }
                _list_it.set(&_curr->value);
            }
            // Else the list iterator has already done a next
            return true;
        }
    };

    template <typename K>
    class IndexRangeNeqReverse_NodeIteratorImpl : public Index_NodeIteratorImplBase {
        typedef List<Node *> IndexValue;
        typedef AvlTreeIndex<K, IndexValue> IndexNode;
        IndexNode *_tree;
        K _neq;
        typename IndexNode::TreeNode *_curr;
        std::stack<typename IndexNode::TreeNode *> _path;

    public:
        IndexRangeNeqReverse_NodeIteratorImpl(IndexNode *tree, const K &neq)
            : Index_NodeIteratorImplBase(NULL),
              _tree(tree), _neq(neq), _curr(NULL)
        {
            // Get to the minimum of the tree but make sure that is
            // not the key itself
            _tree->add_nodes_neq_reverse(tree->_tree, _neq, _path);
            if (!_path.empty()) {
                _curr = _path.top();
                _path.pop();
            }
            if (_curr)
                _list_it.set(&_curr->value);
        }

        bool next() {
            if(!_list_it.next()) { // current list iterator empty
                typename IndexNode::TreeNode *temp = _curr->left;
                _curr = NULL;
                _tree->add_nodes_neq_reverse(temp, _neq, _path);
                if (!_path.empty()) {
                    _curr = _path.top();
                    _path.pop();
                }
                if (!_curr) { // Check if something returned from stack
                    _list_it.set(NULL);
                    return false;
                }
                _list_it.set(&_curr->value);
            }
            // Else the list iterator has already done a next
            return true;
        }
    };
}

template <typename K, typename V>
NodeIterator AvlTreeIndex<K,V>::get_nodes(bool reverse)
{
    if (!reverse)
        return NodeIterator(new IndexRangeNomax_NodeIteratorImpl<K>(this));
    else
        return NodeIterator(new IndexRangeNomin_NodeIteratorImpl<K>(this));
}

template <typename K, typename V>
NodeIterator AvlTreeIndex<K,V>::get_nodes(const K &key, PropertyPredicate::op_t op, bool reverse)
{
    NodeIteratorImplIntf *impl = NULL;
    switch (op) {
        case PropertyPredicate::eq:
            impl = new IndexEq_NodeIteratorImpl<K>(this, key);
            break;
        case PropertyPredicate::ne:
            if (!reverse)
                impl = new IndexRangeNeq_NodeIteratorImpl<K>(this, key);
            else
                impl = new IndexRangeNeqReverse_NodeIteratorImpl<K>(this, key);
            break;
        // < or <= some max. But start from min of tree.
        case PropertyPredicate::lt:
            if (!reverse)
                impl = new IndexRange_NodeIteratorImpl<K>(this, key, false);
            else
                impl = new IndexRangeNomin_NodeIteratorImpl<K>(this, key, false);
            break;
        case PropertyPredicate::le:
            if (!reverse)
                impl = new IndexRange_NodeIteratorImpl<K>(this, key, true);
            else
                impl = new IndexRangeNomin_NodeIteratorImpl<K>(this, key, true);
            break;
        // > or >= some min. But go till the max of tree.
        case PropertyPredicate::gt:
            if (!reverse)
                impl = new IndexRangeNomax_NodeIteratorImpl<K>(this, key, false);
            else
                impl = new IndexRangeReverse_NodeIteratorImpl<K>(this, key, false);
            break;
        case PropertyPredicate::ge:
            if (!reverse)
                impl = new IndexRangeNomax_NodeIteratorImpl<K>(this, key, true);
            else
                impl = new IndexRangeReverse_NodeIteratorImpl<K>(this, key, true);
            break;
        default: // Since Index already checks ops, this shouldn't happen.
            throw Exception(invalid_id);
    }

    return NodeIterator(impl);
}

template <typename K, typename V>
NodeIterator AvlTreeIndex<K,V>::get_nodes(const K &min, const K& max,
                                          PropertyPredicate::op_t op,
                                          bool reverse)
{
    bool incl_min = true, incl_max = true;

    if (op == PropertyPredicate::gtlt) {
        incl_min = false;
        incl_max = false;
    }
    else if (op == PropertyPredicate::gelt)
        incl_max = false;
    else if (op == PropertyPredicate::gtle)
        incl_min = false;

    if (!reverse)
        return NodeIterator(new IndexRange_NodeIteratorImpl<K>(this, min, max, incl_min, incl_max));
    else
        return NodeIterator(new IndexRangeReverse_NodeIteratorImpl<K>(this, min, max, incl_min, incl_max));
}

// Explicitly instantiate any types that might be required
template class AvlTreeIndex<long long, List<Node *>>;
template class AvlTreeIndex<bool, List<Node *>>;
template class AvlTreeIndex<double, List<Node *>>;
template class AvlTreeIndex<IndexString, List<Node *>>;
