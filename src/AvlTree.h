#pragma once
#include <set>
#include "allocator.h"

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
    template<typename T> class AvlTree;

    template<typename T> class AvlTree {
    public: // for testing
            // Max 32B size
            struct AvlTreeNode {
                // Weird ordering to avoid compiler rounding and easier logging
                AvlTreeNode *left;
                AvlTreeNode *right;
                int height;
                T data;
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
            AvlTreeNode *add_recursive(AvlTreeNode *curr, T &data, Allocator &allocator);
            AvlTreeNode *remove_recursive(AvlTreeNode *curr, T &data, Allocator &allocator);
            void add_left_tree(std::set<AvlTreeNode *> &range, AvlTreeNode *curr,
                    T &min, bool inclusive);
            void add_right_tree(std::set<AvlTreeNode *> &range, AvlTreeNode *curr,
                    T &max, bool inclusive, bool *done);

    public:
            AvlTree() : _tree(NULL), _num_elems(0) {}
            size_t num_elems() { return _num_elems; }
            void add(T &data, Allocator &allocator);
            void remove(T &data, Allocator &allocator);
            AvlTreeNode *find(T &data);
            void find_range(std::set<AvlTreeNode *> &r, T &min, T &max, bool inclusive);
            AvlTreeNode *find(const char *name);
            AvlTreeNode *begin() { return _tree; }
            AvlTreeNode *left(const AvlTreeNode *curr) { return (curr == NULL) ? NULL : curr->left; }
            AvlTreeNode *right(const AvlTreeNode *curr) { return (curr == NULL) ? NULL : curr->right; }
            T *data(AvlTreeNode *curr) { return (curr == NULL) ? NULL : &curr->data; }
            // TODO public, only for debugging
            int height(AvlTreeNode *node)
            {
                if (node == NULL)
                    return -1;
                return node->height;
            }
    };
}
