/**
 * @file   AvlTree.h
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

#pragma once
#include "Allocator.h"

// AVL Trees are very good for search which will hopefully be the
// primary function when used for indexing. The balancing condition
// is strict but search complexity remains at O(log n). The remove
// function can cause multiple traversals through the tree but in order
// to maintain correct balance, there is sadly no optimization. If deletes
// become very frequent, consider splay tree or some other data structure.
// Also, from some simple tests, it does seem like balancing starts
// to become less frequent as the tree grows larger.

namespace PMGD {
    class TransactionImpl;
    template<typename K, typename V> class AvlTree {
    protected:
        struct TreeNode {
            // Weird ordering to avoid compiler rounding and easier logging
            TreeNode *left;
            TreeNode *right;
            int height;
            K key;
            V value;
        };

        TreeNode *_tree;
        size_t _num_elems;

    private:
        TreeNode *find_max(TreeNode *t) {
            if (t == NULL || t->right == NULL)
                return t;
            return find_max(t->right);
        }

        TreeNode *left_rotate(TreeNode *hinge, TransactionImpl *tx);
        TreeNode *right_rotate(TreeNode *hinge, TransactionImpl *tx);
        TreeNode *leftright_rotate(TreeNode *hinge, TransactionImpl *tx);
        TreeNode *rightleft_rotate(TreeNode *hinge, TransactionImpl *tx);
        int max(int val1, int val2) { return (val1 > val2) ? val1 : val2; }
        TreeNode *add_recursive(TreeNode *curr, const K &data, V*&r,
                                Allocator &allocator, TransactionImpl *tx,
                                bool &rebalanced);
        TreeNode *remove_recursive(TreeNode *curr, const K &data,
                                   Allocator &allocator, TransactionImpl *tx,
                                   bool &rebalanced);

        int height(TreeNode *node)
        {
            if (node == NULL)
                return -1;
            return node->height;
        }

        // Unit test and associated funtions
        friend class AvlTreeTest;
        TreeNode *left(const TreeNode *curr) { return (curr == NULL) ? NULL : curr->left; }
        TreeNode *right(const TreeNode *curr) { return (curr == NULL) ? NULL : curr->right; }
        K *key(TreeNode *curr) { return (curr == NULL) ? NULL : &curr->key; }
        V *value(TreeNode *curr) { return (curr == NULL) ? NULL : &curr->value; }

    public:
        AvlTree() : _tree(NULL), _num_elems(0) { }

        size_t num_elems() const { return _num_elems; }

        // We could use a key value pair in the tree struct and return a pointer to
        // that but technically, the user shouldn't be allowed to modify anything
        // other than what goes in the value area.
        V *add(const K &key, Allocator &allocator);

        // In most cases, remove will remove the tree node. In cases where user
        // wants to modify the value, it should be a find and modify, which
        // might cause value to become empty, hence triggering a tree node delete.
        // That would mean a repeat search for the node but I don't see another
        // cleaner option.
        void remove(const K &key, Allocator &allocator);

        V *find(const K &key);

        size_t treenode_size(TreeNode *node);
    };
}
