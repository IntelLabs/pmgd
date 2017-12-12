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
size_t AvlTree<K,V>::num_elems_recursive(AvlTree<K,V>::TreeNode *node,
                                       TransactionImpl *tx) const
{
    size_t count = 1;

    // The parent is locked. So this won't change.
    if (node == NULL)
        return 0;
    tx->acquire_lock(TransactionImpl::IndexLock, node, false);
    count += num_elems_recursive(node->left, tx);
    count += num_elems_recursive(node->right, tx);
    return count;
}

template <typename K, typename V>
size_t AvlTree<K,V>::num_elems() const
{
    TransactionImpl *tx = TransactionImpl::get_tx();
    tx->acquire_lock(TransactionImpl::IndexLock, this, false);
    if (_tree == NULL)
        return 0;
    tx->acquire_lock(TransactionImpl::IndexLock, _tree, false);
    return num_elems_recursive(_tree->left, tx) + num_elems_recursive(_tree->right, tx) + 1;
}

template <typename K, typename V>
int AvlTree<K,V>::get_locks_add_recursive(AvlTree<K,V>::TreeNode * const curr, const K &key,
                                                std::vector<LockInfo> &mylocks,
                                                TransactionImpl *tx,
                                                int &r, const unsigned level)
{
    int lefth, righth;

    if (curr == NULL) {
        mylocks.push_back({NULL, 0, true, true, true});
        return 0;
    }

    tx->acquire_lock(TransactionImpl::IndexLock, curr, false);

    mylocks.push_back({curr, curr->height, false, false, false});

    if (key == curr->key) {
        r = level;  // indicate the level where the key was found directly to avoid re-traversal
        return -1;
    }
    if (key < curr->key) {
        lefth = get_locks_add_recursive(curr->left, key, mylocks, tx, r, level + 1);
        if (lefth == -1)  // no new node needed. Key matched.
            return -1;

        // Need to lock the right child before reading its height
        if (curr->right != NULL) {
            tx->acquire_lock(TransactionImpl::IndexLock, curr->right, false);
            righth = curr->right->height;  // Since we went left, this didn't change
        }
        else
            righth = -1;
    }
    else {
        righth = get_locks_add_recursive(curr->right, key, mylocks, tx, r, level + 1);
        if (righth == -1)  // no new node needed. Key matched.
            return -1;

        // Need to lock the left child before reading its height
        if (curr->left != NULL) {
            tx->acquire_lock(TransactionImpl::IndexLock, curr->left, false);
            lefth = curr->left->height;  // Since we went right, this didn't change
        }
        else
            lefth = -1;
    }

    mylocks[level].myht = max(lefth, righth) + 1;  // rotate might change this.

    // check for imbalance > 1: |lefth - righth| == 2
    if ((uint(lefth - righth) & 3) == 2) {
        mylocks[level].myht = curr->height;
        mylocks[level].ptr_change = true;
        mylocks[level].ptr_change_for_parent = true;
    } else if (mylocks[level + 1].ptr_change_for_parent)
        mylocks[level].ptr_change = true;

    // If there was no rotation at a level but the lower level completely changed,
    // then we need to capture that.
    if (curr->height != mylocks[level].myht)
        mylocks[level].ht_change = true;

    if (mylocks[level].ht_change || mylocks[level].ptr_change)
        r = (level == 0) ? level : level - 1;

    return mylocks[level].myht;
}

template <typename K, typename V>
int AvlTree<K,V>::get_locks_add(const K &key, AvlTree<K,V>::TreeNode **to_lock,
                                          TransactionImpl *tx)
{
    // Acquire a read lock before descending into the tree structure otherwise
    // someone might modify the root itself.
    // TODO Some commmented out optimizations since the journal fills
    // up inexplicably.
    if (tx->acquire_lock(TransactionImpl::IndexLock, this, false)
                == TransactionImpl::WriteLock)
        return 1;

    // Lock before descending into the tree. Avoid lock checking if
    // we also already hold a lock to _tree.
    if (_tree == NULL) {
        // If this TX has write locked _tree, it won't hurt much if
        // we just locked the entire index.
        tx->acquire_lock(TransactionImpl::IndexLock, this, true);
        return 1;
    }

    // Lock before descending into the tree. Avoid lock checking if
    // we also already hold a lock to _tree.
    if (tx->acquire_lock(TransactionImpl::IndexLock, _tree, false)
            == TransactionImpl::WriteLock) {
        // If this TX has write locked _tree, it won't hurt much if
        // we just locked the entire index.
        tx->acquire_lock(TransactionImpl::IndexLock, this, true);
        return 1;
    }

    std::vector<LockInfo> scanlocks;
    int r = 0;

    // In case a match is found, return which level of the vector or which
    // is the highest level that sees an update.
    int rel_lvl = 0;

    // To avoid the expensive reallocations that can happen for a vector,
    // compute the max we could need. Wastes memory but won't cause slowdown.
    scanlocks.reserve(StripedLock::ceiling_log2(_tree->height) + 1);
    r = get_locks_add_recursive(_tree, key, scanlocks, tx, rel_lvl, 0);

    if (rel_lvl == 0) {
        // This happens when the root is getting downgraded. So that
        // is where to_lock needs to be and there is no need to lock
        // any other tree node anymore.
        tx->acquire_lock(TransactionImpl::IndexLock, this, true);
        *to_lock = _tree;
        return 1;
    }

    // level either contains level of tree where a match was found or how
    // high up did updates reach. We need to write lock in either cases.
    // Given this is an add call, makes sense to write lock the node that
    // matched the key.
    *to_lock = scanlocks[rel_lvl].curr;
    tx->acquire_lock(TransactionImpl::IndexLock, *to_lock, true);

    // Even for a match, let's not release the read locks along the path
    // as of now since the thread needs to ensure it is not releasing
    // anything it had locked the previous iteration and so on. Also,
    // releasing these read locks will mean changing delete to ensure
    // the replace node along the delete path is not write locked.

    return r;
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
    TreeNode *to_lock = NULL;
    int retval;

    retval = get_locks_add(key, &to_lock, tx);
    if (retval < 0)
        r = &(to_lock->value);
    else {
        bool rebalanced = false;
        if (retval == 1) {
            tx->log(&_tree, sizeof(_tree));
            _tree = add_recursive(_tree, key, r, allocator, tx, rebalanced);
        }
        else {
            // At this stage, we know that the pointer pointed to by to_lock
            // has some of its contents modified but the pointer itself remains
            // fine. So no need to log anything there. The contents will get
            // appropriately logged inside add.
            to_lock = add_recursive(to_lock, key, r, allocator, tx, rebalanced);
        }

        if (rebalanced)
            tx->iterator_callbacks().iterator_rebalance_notify(this);
    }
    return r;
}

template <typename K, typename V>
typename AvlTree<K,V>::TreeNode *AvlTree<K,V>::get_locks_find_max(TreeNode *t,
                                        TransactionImpl *tx)
{
    if (t == NULL || t->right == NULL)
        return t;
    tx->acquire_lock(TransactionImpl::IndexLock, t->right, false);
    return get_locks_find_max(t->right, tx);
}

template <typename K, typename V>
int AvlTree<K,V>::get_locks_remove_recursive(AvlTree<K,V>::TreeNode * const curr,
                                                const K &key,
                                                std::vector<LockInfo> &mylocks,
                                                TransactionImpl *tx,
                                                AvlTree<K,V>::TreeNode **to_delete, // Node that actually gets deleted
                                                int &r,
                                                const unsigned level)
{
    int lefth, righth;

    // Key not in tree. Nothing to lock.
    if (curr == NULL)
        return -1;
    tx->acquire_lock(TransactionImpl::IndexLock, curr, false);

    if (key > curr->key) { // Change in right subtree
        mylocks.push_back({curr, curr->height, false, false, false});
        if (get_locks_remove_recursive(curr->right, key, mylocks, tx, to_delete, r, (level + 1)) < 0)
            return -1;
        righth = mylocks[level + 1].myht;
        if (curr->left != NULL) {
            tx->acquire_lock(TransactionImpl::IndexLock, curr->left, false);
            lefth = curr->left->height;
        }
        else
            lefth = -1;
        mylocks[level].myht = max(lefth, righth) + 1;
        if (mylocks[level + 1].ptr_change_for_parent)
            mylocks[level].ptr_change = true;
        if (lefth - righth == 2) {
            // Check the balance of the other subtree. If it is balanced,
            // then no height change here.
            // Left tree was not affected till now. So it is ok to check its pointers.
            // And we know that the left tree is not null because of the height check
            // earlier.
            if (curr->left->right != NULL)
                tx->acquire_lock(TransactionImpl::IndexLock, curr->left->right, false);
            if (curr->left->left != NULL)
                tx->acquire_lock(TransactionImpl::IndexLock, curr->left->left, false);
            if (height(curr->left->right) == height(curr->left->left))
                mylocks[level].myht = curr->height;
            // Case for double rotation or single rotate with unbalanced tree.
            // It always reduces height at curr and causes a pointer change in
            // the upper level
            else {
                mylocks[level].myht = curr->height - 1;
                mylocks[level].ht_change = true;
            }
            mylocks[level].ptr_change_for_parent = true;
        }
    }
    else { // <=
        if (key == curr->key) {
            r = level;  // Level where the real key was.
            // If no children or just one child, just free given node and return
            // non-empty child or NULL
            if (curr->left == NULL || curr->right == NULL) {
                TreeNode *temp = curr->left != NULL ? curr->left : curr->right;
                int ht = -1;
                if (temp != NULL) {
                    tx->acquire_lock(TransactionImpl::IndexLock, temp, false);
                    ht = temp->height;
                }
                mylocks.push_back({curr, ht, true, true, true});
                return 0;
            }
            mylocks.push_back({curr, curr->height, false, true, false});

            // curr has both children.
            // Find its in-order predecessor to exchange data,value with. This
            // predecessor will then be the actual deleted node.
            // But we will have to run delete again and traverse again cause
            // the heights and rotations need to be done correctly.
            tx->acquire_lock(TransactionImpl::IndexLock, curr->left, false);
            TreeNode *to_replace = get_locks_find_max(curr->left, tx);
            *to_delete = to_replace;
            int r_discard;

            // No need to check return value here
            get_locks_remove_recursive(curr->left, to_replace->key, mylocks, tx, to_delete, r_discard, (level + 1));
        }
        else if (key < curr->key) { // Change in left subtree
            mylocks.push_back({curr, curr->height, false, false, false});
            if (get_locks_remove_recursive(curr->left, key, mylocks, tx, to_delete, r, (level + 1)) < 0)
                return -1;
        }

        lefth = mylocks[level + 1].myht;
        if (curr->right != NULL) {
            tx->acquire_lock(TransactionImpl::IndexLock, curr->right, false);
            righth = curr->right->height;
        }
        else
            righth = -1;
        mylocks[level].myht = max(lefth, righth) + 1;
        if (mylocks[level + 1].ptr_change_for_parent)
            mylocks[level].ptr_change = true;
        if (lefth - righth == -2) {
            // Check the balance of the other subtree. If it is balanced,
            // then no height change here.
            // Right tree was not affected till now. So it is ok to check its pointers.
            // And we know that the right tree is not null because of the height check
            // earlier.
            if (curr->right->right != NULL)
                tx->acquire_lock(TransactionImpl::IndexLock, curr->right->right, false);
            if (curr->right->left != NULL)
                tx->acquire_lock(TransactionImpl::IndexLock, curr->right->left, false);
            if (height(curr->right->right) == height(curr->right->left))
                mylocks[level].myht = curr->height;
            // Case for double rotation or single rotate with unbalanced tree.
            // It always reduces height at curr and causes a pointer change in
            // the upper level
            else {
                mylocks[level].myht = curr->height - 1;
                mylocks[level].ht_change = true;
            }
            mylocks[level].ptr_change_for_parent = true;
        }  // rebalance
    }
    
    // If there was no rotation at a level but the lower level completely changed,
    // then we need to capture that.
    if (curr->height != mylocks[level].myht)
        mylocks[level].ht_change = true;

    return 0;
}

template <typename K, typename V>
int AvlTree<K,V>::get_locks_remove(const K &key, AvlTree<K,V>::TreeNode **to_lock,
                                          AvlTree<K,V>::TreeNode **to_delete,
                                          AvlTree<K,V>::TreeNode **to_change,
                                          TransactionImpl *tx)
{
    // Acquire a read lock before descending into the tree structure otherwise
    // someone might modify the root itself.
    // TODO Some commmented out optimizations since the journal fills
    // up inexplicably.
    if (tx->acquire_lock(TransactionImpl::IndexLock, this, false)
                == TransactionImpl::WriteLock) {
        *to_lock = _tree;
        return (_tree == NULL) ? -1 : 0;
    }

    // Lock before descending into the tree. Avoid lock checking if
    // we also already hold a lock to _tree.
    if (_tree == NULL)
        return -1;

    // Lock before descending into the tree. Avoid lock checking if
    // we also already hold a lock to _tree.
    if (tx->acquire_lock(TransactionImpl::IndexLock, _tree, false)
            == TransactionImpl::WriteLock) {
        // If this TX has write locked _tree, it won't hurt much if
        // we just locked the entire index.
        tx->acquire_lock(TransactionImpl::IndexLock, this, true);
        *to_lock = _tree;
        return 0;
    }

    std::vector<LockInfo> scanlocks;

    // To avoid the expensive reallocations that can happen for a vector,
    // compute the max we could need. Wastes memory but won't cause slowdown.
    scanlocks.reserve(StripedLock::ceiling_log2(_tree->height));

    int level; // In case a match is found, return which level of the vector.

    if (get_locks_remove_recursive(_tree, key, scanlocks, tx, to_delete, level, 0) < 0)
        return -1;
    else {
        // Which level is the replacement meant to be
        if (*to_delete != NULL)
            *to_change = scanlocks[level].curr;

        // Get the stripe order to go ahead and lock.
        for (unsigned i = 0; i < scanlocks.size(); ++i) {
            LockInfo &info = scanlocks[i];
            if (info.ht_change || info.ptr_change) {
                // We are overestimating by 2 levels instead of 1 in case the
                // estimate calculation is off for some reason.
                // *** rare bug but not sure what is causing it.
                if (i <= 2) {
                    *to_lock = _tree;
                    tx->acquire_lock(TransactionImpl::IndexLock, this, true);
                    tx->acquire_lock(TransactionImpl::IndexLock, _tree, true);
                    break;
                }
                tx->acquire_lock(TransactionImpl::IndexLock, scanlocks[i - 2].curr, true);
                *to_lock = scanlocks[i - 2].curr;

                // No need to lock any further
                break;
            }
        }
    }

    return 0;
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
int AvlTree<K,V>::remove(const K &key, Allocator &allocator)
{
    TransactionImpl *tx = TransactionImpl::get_tx();
    bool rebalanced = false;

    // to_change represents the node that will actually be changed and has
    // the key that needs to be deleted. But this node may not actually
    // be removed from the tree. Only its key and value might be replaced
    // but the key and value from to_delete.
    // to_delete is the node of the tree that is going to get deleted
    // but its key will be moved into the node represented by to_change.
    TreeNode *to_delete = NULL, *to_change = NULL, *to_lock;
    if (get_locks_remove(key, &to_lock, &to_delete, &to_change, tx) < 0)
        return -1;

    if (to_lock == _tree) {
        tx->log(&_tree, sizeof(_tree));
        _tree = remove_recursive(_tree, key, allocator, tx, rebalanced);
    }
    else {
        if (to_delete != NULL && to_change == to_lock) {
            // This node will never be NULL.
            tx->log(to_lock, sizeof *to_lock);
            to_lock->key = to_delete->key;
            to_lock->value = to_delete->value;
            tx->log(&to_lock->left, sizeof(to_lock->left));
            to_lock->left = remove_recursive(to_lock->left, to_delete->key, allocator, tx, rebalanced);
        }
        else {
            // At this stage, we know that the pointer pointed to by to_lock
            // has some of its contents modified but the pointer itself remains
            // fine. So no need to log anything there. The contents will get
            // appropriately logged inside add.
            to_lock = remove_recursive(to_lock, key, allocator, tx, rebalanced);
        }
    }

    if (rebalanced)
        tx->iterator_callbacks().iterator_rebalance_notify(this);
    return 0;
}

template <typename K, typename V>
void AvlTree<K,V>::get_locks_find(const K &key, AvlTree::TreeNode **found)
{
    TransactionImpl *tx = TransactionImpl::get_tx();
    tx->acquire_lock(TransactionImpl::IndexLock, this, false);
    TreeNode *curr = _tree;
    while (curr != NULL) {
        tx->acquire_lock(TransactionImpl::IndexLock, curr, false);
        if (key == curr->key) {
            *found = curr;
            return;
        }
        else if (key < curr->key)
            curr = curr->left;
        else
            curr = curr->right;
    }
}

template <typename K, typename V>
V *AvlTree<K,V>::find(const K &key, bool write_lock_tree_node)
{
    TreeNode *curr = NULL;

    get_locks_find(key, &curr);

    if (curr != NULL) {
        if (write_lock_tree_node) {
            TransactionImpl *tx = TransactionImpl::get_tx();
            tx->acquire_lock(TransactionImpl::IndexLock, curr, true);
        }
        return &(curr->value);
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
