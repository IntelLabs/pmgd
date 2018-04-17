/**
 * @file   AvlTree.cc
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

#include <string.h>    // For memset
#include "AvlTree.h"
#include "node.h"
#include "List.h"
#include "IndexString.h"

using namespace PMGD;

// hinge-->new_root-->its left and right children
// The return value of this function gets assigned to
// the parent of hinge that will call this function
// Also called rotate_with_right
template <typename K, typename V>
typename AvlTree<K,V>::TreeNode *AvlTree<K,V>::left_rotate(
                                       AvlTree<K,V>::TreeNode *hinge,
                                       TransactionImpl *tx)
{
    TreeNode *new_root = hinge->right;
    // We change only left and height for new node but right is in between.
    tx->log_range(&new_root->left, &new_root->height);
    // We change only right and height for the hinge.
    tx->log_range(&hinge->right, &hinge->height);

    hinge->right = new_root->left;
    hinge->height = max(height(hinge->left), height(hinge->right)) + 1;
    new_root->left = hinge;
    // TODO this might be a repeat given what follows in the caller
    // but for ensuring correct heights, lets set this anyway.
    new_root->height = max(height(hinge), height(new_root->right)) + 1;
    return new_root;
}

// its left right children<--new_root<--hinge
// The return value of this function gets assigned to
// the parent of hinge that will call this function
// Also called rotate_with_left
template <typename K, typename V>
typename AvlTree<K,V>::TreeNode *AvlTree<K,V>::right_rotate(
                                       AvlTree<K,V>::TreeNode *hinge,
                                       TransactionImpl *tx)
{
    TreeNode *new_root = hinge->left;
    // We change only right and height for the new root.
    tx->log_range(&new_root->right, &new_root->height);
    // We change only left and height but right is in the middle.
    tx->log_range(&hinge->left, &hinge->height);

    hinge->left = new_root->right;
    hinge->height = max(height(hinge->left), height(hinge->right)) + 1;
    new_root->right = hinge;
    // TODO this might be a repeat given what follows in the caller
    // but for ensuring correct heights, lets set this anyway.
    new_root->height = max(height(hinge), height(new_root->left)) + 1;
    return new_root;
}

/* a
    \
     c
    /
   b
   Also called double left
*/
template <typename K, typename V>
typename AvlTree<K,V>::TreeNode *AvlTree<K,V>::leftright_rotate(
                                       AvlTree<K,V>::TreeNode *hinge,
                                       TransactionImpl *tx)
{
    tx->log(&hinge->right, sizeof(hinge->right));
    hinge->right = right_rotate(hinge->right, tx);
    return left_rotate(hinge, tx);
}

/*   c
    /
   a
    \
     b
   Also called double right
*/
template <typename K, typename V>
typename AvlTree<K,V>::TreeNode *AvlTree<K,V>::rightleft_rotate(
                                       AvlTree<K,V>::TreeNode *hinge,
                                       TransactionImpl *tx)
{
    tx->log(&hinge->left, sizeof(hinge->left));
    hinge->left = left_rotate(hinge->left, tx);
    return right_rotate(hinge, tx);
}

template <typename K, typename V>
typename AvlTree<K,V>::TreeNode *AvlTree<K,V>::add_recursive(AvlTree<K,V>::TreeNode *curr,
                                                const K &key, V *&r, Allocator &allocator,
                                                TransactionImpl *tx, bool &rebalanced)
{
    if (curr == NULL) {
        TreeNode *temp = (TreeNode *)allocator.alloc(sizeof(TreeNode));
        new (&temp->key) K(key);
        r = new (&temp->value) V();
        temp->height = 0;
        temp->left = NULL;
        temp->right = NULL;

        // Since new_node is new allocation, just flush it without logging.
        tx->flush_range(temp, sizeof *temp);
        tx->write(&_num_elems, _num_elems + 1);
        return temp;
    }
    if (key == curr->key) {
        r = &(curr->value);
        return curr;
    }
    else if (key < curr->key) {
        // Log only the left pointer because the rotates might log a lot
        // more with some overlap
        tx->log(&curr->left, sizeof(curr->left));
        curr->left = add_recursive(curr->left, key, r, allocator, tx, rebalanced);
        if (height(curr->left) - height(curr->right) == 2) { // intolerable imbalance
            if (key > curr->left->key)
                curr = rightleft_rotate(curr, tx);
            else
                curr = right_rotate(curr, tx);
            rebalanced = true;
        }
    }
    else {
        // Log only the right pointer because the rotates might log a lot
        // more with some overlap
        tx->log(&curr->right, sizeof(curr->right));
        curr->right = add_recursive(curr->right, key, r, allocator, tx, rebalanced);
        if (height(curr->left) - height(curr->right) == -2) { // intolerable imbalance
            if (key < curr->right->key)
                curr = leftright_rotate(curr, tx);
            else
                curr = left_rotate(curr, tx);
            rebalanced = true;
        }
    }

    // The rotates may or may not have happened and hence the height
    // may or may not have been logged. So make sure it is logged here
    // only if the value changes.
    uint32_t new_height = max(height(curr->left), height(curr->right)) + 1;
    if (new_height != curr->height) {
        tx->log(&curr->height, sizeof(curr->height));
        curr->height = new_height;
    }
    return curr;
}

template <typename K, typename V>
V *AvlTree<K,V>::add(const K &key, Allocator &allocator)
{
    V *r = NULL;
    TransactionImpl *tx = TransactionImpl::get_tx();
    bool rebalanced = false;
    tx->log(&_tree, sizeof(_tree));
    _tree = add_recursive(_tree, key, r, allocator, tx, rebalanced);

    if (rebalanced)
        tx->get_db()->index_manager().iterator_rebalance_notify(this);

    return r;
}

template <typename K, typename V>
typename AvlTree<K,V>::TreeNode *AvlTree<K,V>::remove_recursive(AvlTree<K,V>::TreeNode *curr,
                                                const K &key, Allocator &allocator,
                                                TransactionImpl *tx, bool &rebalanced)
{
    if (curr == NULL)
        return NULL;

    if (key > curr->key) {
        // Log only the right pointer because the rotates might log a lot
        // more with some overlap
        tx->log(&curr->right, sizeof(curr->right));
        curr->right = remove_recursive(curr->right, key, allocator, tx, rebalanced);
        // Left tree too large?
        if (height(curr->left) - height(curr->right) == 2) {
            // Find taller child of left child now. Left is clearly not null
            if (height(curr->left->left) > height(curr->left->right))
                curr = right_rotate(curr, tx);
            else
                curr = rightleft_rotate(curr, tx);
            rebalanced = true;
        }
    }
    else {
        if (key == curr->key) {
            // If no children or just one child, just free given node and return
            // non-empty child or NULL
            if (curr->left == NULL || curr->right == NULL) {
                TreeNode *temp = curr->left != NULL ? curr->left : curr->right;
                curr->key.~K();
                // The value node gets destroyed by the caller.
                // So the tree code doesn't need to free any pointers there.
                allocator.free(curr, sizeof *curr);
                tx->write(&_num_elems, _num_elems - 1);
                // This node's and its own childrens' heights won't get affected
                // by this step. So return a level up where the parent will be go
                // through balancing check
                return temp;
            }
            // curr has both children.
            // Find its in-order predecessor to exchange data,value with. This
            // predecessor will then be the actual deleted node.
            // But we will have to run delete again and traverse again cause
            // the heights and rotations need to be done correctly.
            TreeNode *to_replace = find_max(curr->left);
            // This node will never be NULL.
            tx->log(curr, sizeof *curr);
            curr->key = to_replace->key;
            curr->value = to_replace->value;
            tx->log(&curr->left, sizeof(curr->left));
            curr->left = remove_recursive(curr->left, to_replace->key, allocator, tx, rebalanced);
        }
        else {
            // Log only the left pointer because the rotates might log a lot
            // more with some overlap
            tx->log(&curr->left, sizeof(curr->left));
            curr->left = remove_recursive(curr->left, key, allocator, tx, rebalanced);
        }
        // Right tree too large?
        if (height(curr->left) - height(curr->right) == -2) {
            // Find tallest child of right tree for rotate. Right tree is not null.
            if (height(curr->right->left) > height(curr->right->right))
                curr = leftright_rotate(curr, tx);
            else
                curr = left_rotate(curr, tx);
            rebalanced = true;
        }
    }

    // The rotates may or may not have happened and hence the height
    // may or may not have been logged. So make sure it is logged here
    // only if the value changes.
    uint32_t new_height = max(height(curr->left), height(curr->right)) + 1;
    if (new_height != curr->height) {
        tx->log(&curr->height, sizeof(curr->height));
        curr->height = new_height;
    }
    return curr;
}

template <typename K, typename V>
void AvlTree<K,V>::remove(const K &key, Allocator &allocator)
{
    TransactionImpl *tx = TransactionImpl::get_tx();
    bool rebalanced = false;
    tx->log(&_tree, sizeof(_tree));
    _tree = remove_recursive(_tree, key, allocator, tx, rebalanced);

    if (rebalanced)
        tx->get_db()->index_manager().iterator_rebalance_notify(this);
}

template <typename K, typename V>
V *AvlTree<K,V>::find(const K &key)
{
    TreeNode *curr = _tree;
    while (curr != NULL) {
        if (key == curr->key)
            return &(curr->value);
        else if (key < curr->key)
            curr = curr->left;
        else
            curr = curr->right;
    }
    return NULL;
}

template <typename K, typename V>
size_t AvlTree<K,V>::treenode_size(TreeNode *node)
{
    return sizeof(*node);
}

namespace PMGD {
    // Specialization for the IndexString case
    template <>
    size_t AvlTree<IndexString, List<void *> >::treenode_size(TreeNode *node)
    {
        return sizeof(*node) + node->key.get_remainder_size();
    }
}

// Explicitly instantiate any types that might be required
template class AvlTree<int, int>;
template class AvlTree<long long, List<void *>>;
template class AvlTree<bool, List<void *>>;
template class AvlTree<double, List<void *>>;
template class AvlTree<Time, List<void *>>;
template class AvlTree<IndexString, List<void *>>;
