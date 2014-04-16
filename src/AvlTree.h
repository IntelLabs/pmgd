#pragma once
#include "allocator.h"
#include "KeyValuePair.h"

// AVL Trees are very good for search which will hopefully be the
// primary function when used for indexing. The balancing condition
// is strict but search complexity remains at O(log n). The remove
// function can cause multiple traversals through the tree but in order
// to maintain correct balance, there is sadly no optimization. If deletes
// become very frequent, consider splay tree or some other data structure.
// Also, from some simple tests, it does seem like balancing starts
// to become less frequent as the tree grows larger.

// While most of our use of this tree will be with separate keys and values
// just use one template parameter and pass the KeyValue data structure. That
// way we can use these trees just to store singular entries with no values.
namespace Jarvis {
    template<typename K, typename V> class AvlTree;

    template<typename K, typename V> class AvlTree {
    public: // for testing
            // Max 32B size
            struct AvlTreeNode {
                // Weird ordering to avoid compiler rounding and easier logging
                AvlTreeNode *left;
                AvlTreeNode *right;
                int height;
                K key;
                V value;
            };

    private:
            AvlTreeNode *_tree;
            size_t _num_elems;

            AvlTreeNode *find_max(AvlTreeNode *t) {
                if (t == NULL || t->right == NULL)
                    return t;
                return find_max(t->right);
            }

            AvlTreeNode *left_rotate(AvlTreeNode *hinge);
            AvlTreeNode *right_rotate(AvlTreeNode *hinge);
            AvlTreeNode *leftright_rotate(AvlTreeNode *hinge);
            AvlTreeNode *rightleft_rotate(AvlTreeNode *hinge);
            int max(int val1, int val2) { return (val1 > val2) ? val1 : val2; }
            AvlTreeNode *add_recursive(AvlTreeNode *curr, K &data, V**r, Allocator &allocator);
            AvlTreeNode *remove_recursive(AvlTreeNode *curr, K &data, Allocator &allocator);

    public:
            AvlTree() : _tree(NULL), _num_elems(0) {}
            size_t num_elems() { return _num_elems; }
            // We could use a key value pair in the tree struct and return a pointer to
            // that but technically, the user shouldn't be allowed to modify anything
            // other than what goes in the value area.
            V *add(K &key, Allocator &allocator);
            // In most cases, remove will remove the tree node, in cases where user
            // wants to modify the value, it should be a find and modify, which
            // in cases where value becomes empty, could trigger a tree node delete
            // which would mean a repeat search for the node but I don't see another
            // cleaner option. 
            void remove(K &key, Allocator &allocator);
            AvlTreeNode *find(const K &key);
            AvlTreeNode *begin() { return _tree; }
            AvlTreeNode *left(const AvlTreeNode *curr) { return (curr == NULL) ? NULL : curr->left; }
            AvlTreeNode *right(const AvlTreeNode *curr) { return (curr == NULL) ? NULL : curr->right; }
            V *value(AvlTreeNode *curr) { return (curr == NULL) ? NULL : &curr->value; }
            // TODO public, only for debugging
            K *key(AvlTreeNode *curr) { return (curr == NULL) ? NULL : &curr->key; }
            int height(AvlTreeNode *node)
            {
                if (node == NULL)
                    return -1;
                return node->height;
            }
    };
}
