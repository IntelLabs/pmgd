#include "AvlTreeIndex.h"
#include "iterator.h"
#include "List.h"
#include "IndexString.h"

using namespace Jarvis;

namespace Jarvis {
    template <typename K, typename V> class AvlTreeIndex<K,V>::Compare {
        bool _equal;
    public:
        Compare(bool equal) : _equal(equal) {}
        // This is to be used only for making sure the values
        // are not equal. No implications on < or > should be made.
        bool equals(const K &val1, const K &val2) const
            { return (val1 == val2) && _equal; }

        bool lessthan(const K &val1, const K &val2) const
            { return (val1 < val2); }

        bool greaterthan(const K &val1, const K &val2) const
            { return (val1 > val2); }

        bool greaterthanequal(const K &val1, const K &val2) const
            { return (val1 > val2) || equals(val1, val2); }
    };
}

template <typename K, typename V>
void AvlTreeIndex<K,V>::find_start(typename AvlTree<K,V>::TreeNode *root,
                const K &min, const K &max,
                Compare &cmin,
                Compare &cmax,
                std::stack<typename AvlTree<K,V>::TreeNode *> &path)
{
    if (!root)
        return;
    if (cmin.lessthan(min, root->key)) {
        if (cmax.greaterthanequal(max, root->key))
            path.push(root);
        find_start(root->left, min, max, cmin, cmax, path);
    }
    else if (cmin.equals(min, root->key))
            path.push(root);
    else  // Need to look for min in the right subtree
        find_start(root->right, min, max, cmin, cmax, path);
}

// We have already traversed to the min in the tree. So when
// we backtrack through the stack, the only check we need to
// make is that the value <(=) max.
template <typename K, typename V>
void AvlTreeIndex<K,V>::add_right_tree(typename AvlTree<K,V>::TreeNode *root,
                const K &max, Compare &cmax,
                std::stack<typename AvlTree<K,V>::TreeNode *> &path)
{
    if (!root)
        return;
    if (cmax.greaterthanequal(max, root->key))
        path.push(root);
    add_right_tree(root->left, max, cmax, path);
}

namespace Jarvis {
    class IndexEq_NodeIteratorImpl : public NodeIteratorImplIntf {
        ListTraverser<Node *> _it;
    public:
        IndexEq_NodeIteratorImpl(List<Node *> *l)
            : _it(l)
        { }
        Node &operator*() { return **_it; }
        Node *operator->() { return *_it; }
        Node &operator*() const { return **_it; }
        Node *operator->() const { return *_it; }
        operator bool() const { return _it; }
        bool next() { return _it.next(); }
    };

    // TODO: Currently, this is very specific for the V param of
    // the tree. Need to make sure that data structure is flexible
    // and we can accommodate edges easily too.
    template <typename K>
    class IndexRange_NodeIteratorImpl : public NodeIteratorImplIntf {
        typedef List<Node *> IndexValue;
        typedef AvlTreeIndex<K, IndexValue> IndexNode;
        IndexNode *_tree;
        typename IndexNode::TreeNode *_curr;
        ListTraverser<Node *> _list_it;
        K _min, _max;
        typename IndexNode::Compare cmin;
        typename IndexNode::Compare cmax;
        std::stack<typename IndexNode::TreeNode *> _path;

    public:
        IndexRange_NodeIteratorImpl(IndexNode *tree,
                                    const K &min, const K &max,
                                    bool incl_min, bool incl_max)
            : _tree(tree), _curr(NULL), _list_it(NULL),
              _min(min), _max(max),
              cmin(incl_min), cmax(incl_max)
        {
            _tree->find_start(tree->_tree, min, max, cmin, cmax, _path);
            if (!_path.empty()) {
                _curr = _path.top();
                _path.pop();
            }
            if (_curr)
                _list_it.set(&_curr->value);
        }
        Node &operator*() { return **_list_it; }
        Node *operator->() { return *_list_it; }
        Node &operator*() const { return **_list_it; }
        Node *operator->() const { return *_list_it; }
        operator bool() const { return _list_it; }
        bool next() {
            if(!_list_it.next()) { // current list iterator empty
                typename IndexNode::TreeNode *temp = _curr->right;
                _curr = NULL;
                _tree->add_right_tree(temp, _max, cmax, _path);
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
NodeIterator AvlTreeIndex<K,V>::get_nodes(const K &key, const PropertyPredicate &pp)
{
    V *value = this->find(key);
    return NodeIterator(new IndexEq_NodeIteratorImpl(value));
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
