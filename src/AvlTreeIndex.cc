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
    if (!root)
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
    if (!root)
        return;
    if (cmax.greaterthanequal(root->key))
        path.push(root);
    add_right_tree(root->left, cmax, path);
}

// Use this to find first element when no min given
template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start_min(TreeNode *root, const Compare &cmax,
                                       std::stack<TreeNode *> &path)
{
    if (!root)
        return;
    // min is the lowest element in the tree
    if (cmax.greaterthanequal(root->key))
        path.push(root);
    find_start_min(root->left, cmax, path);
}

// Use this to find first element when no max given
template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start_max(TreeNode *root, const Compare &cmin,
                                       std::stack<TreeNode *> &path)
{
    if (!root)
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

// Use this to add all elements when no max limit given
template <typename K, typename V>
void AvlTreeIndex<K,V>::add_full_right_tree(TreeNode *root,
                                            std::stack<TreeNode *> &path)
{
    if (!root)
        return;
    path.push(root);
    add_full_right_tree(root->left, path);
}

// Use this to find first element when no min/max given
template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start_all(TreeNode *root, std::stack<TreeNode *> &path)
{
    if (!root)
        return;
    path.push(root);
    find_start_all(root->left, path);
}

// Use this to find min element of tree that is not equal to given key.
// Same function works for traversing the remaining tree too.
template <typename K, typename V>
void AvlTreeIndex<K,V>::add_nodes_neq(TreeNode *root, const K &neq,
                                      std::stack<TreeNode *> &path)
{
    if (!root)
        return;
    if (!(neq == root->key))
        path.push(root);
    else {
        // If the root == neq, we would miss its right.
        // No need to check for neq cause of unique nodes.
        if (root->right != NULL)
            path.push(root->right);
    }
    add_nodes_neq(root->left, neq, path);
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
}

template <typename K, typename V>
NodeIterator AvlTreeIndex<K,V>::get_nodes()
{
    return NodeIterator(new IndexRangeNomax_NodeIteratorImpl<K>(this));
}

template <typename K, typename V>
NodeIterator AvlTreeIndex<K,V>::get_nodes(const K &key, PropertyPredicate::op_t op)
{
    NodeIteratorImplIntf *impl = NULL;
    switch (op) {
        case PropertyPredicate::eq:
            impl = new IndexEq_NodeIteratorImpl<K>(this, key);
            break;
        case PropertyPredicate::ne:
            impl = new IndexRangeNeq_NodeIteratorImpl<K>(this, key);
            break;
        // < or <= some max. But start from min of tree.
        case PropertyPredicate::lt:
            impl = new IndexRange_NodeIteratorImpl<K>(this, key, false);
            break;
        case PropertyPredicate::le:
            impl = new IndexRange_NodeIteratorImpl<K>(this, key, true);
            break;
        // > or >= some min. But go till the max of tree.
        case PropertyPredicate::gt:
            impl = new IndexRangeNomax_NodeIteratorImpl<K>(this, key, false);
            break;
        case PropertyPredicate::ge:
            impl = new IndexRangeNomax_NodeIteratorImpl<K>(this, key, true);
            break;
        default: // Since Index already checks ops, this shouldn't happen.
            throw Exception(invalid_id);
    }

    return NodeIterator(impl);
}

template <typename K, typename V>
NodeIterator AvlTreeIndex<K,V>::get_nodes(const K &min, const K& max,
                                          PropertyPredicate::op_t op)
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

    return NodeIterator(new IndexRange_NodeIteratorImpl<K>(this, min, max, incl_min, incl_max));
}

// Explicitly instantiate any types that might be required
template class AvlTreeIndex<long long, List<Node *>>;
template class AvlTreeIndex<bool, List<Node *>>;
template class AvlTreeIndex<double, List<Node *>>;
template class AvlTreeIndex<IndexString, List<Node *>>;
