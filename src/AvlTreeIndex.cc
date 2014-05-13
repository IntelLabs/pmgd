#include <string.h>    // For memset
#include "AvlTreeIndex.h"
#include "node.h"
#include "List.h"

using namespace Jarvis;

// hinge-->new_root-->its left and right children
// The return value of this function gets assigned to
// the parent of hinge that will call this function
// Also called rotate_with_right
template <typename K, typename V>
typename AvlTreeIndex<K,V>::TreeNode *AvlTreeIndex<K,V>::left_rotate(AvlTreeIndex<K,V>::TreeNode *hinge)
{
    TreeNode *new_root = hinge->right;
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
typename AvlTreeIndex<K,V>::TreeNode *AvlTreeIndex<K,V>::right_rotate(AvlTreeIndex<K,V>::TreeNode *hinge)
{
    TreeNode *new_root = hinge->left;
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
typename AvlTreeIndex<K,V>::TreeNode *AvlTreeIndex<K,V>::leftright_rotate(AvlTreeIndex<K,V>::TreeNode *hinge)
{
    hinge->right = right_rotate(hinge->right);
    return left_rotate(hinge);
}

/*   c
    /
   a
    \
     b
   Also called double right  
*/   
template <typename K, typename V>
typename AvlTreeIndex<K,V>::TreeNode *AvlTreeIndex<K,V>::rightleft_rotate(AvlTreeIndex<K,V>::TreeNode *hinge)
{
    hinge->left = left_rotate(hinge->left);
    return right_rotate(hinge);
}

template <typename K, typename V>
typename AvlTreeIndex<K,V>::TreeNode *AvlTreeIndex<K,V>::add_recursive(AvlTreeIndex<K,V>::TreeNode *curr,
                                                const K &key, V *&r, Allocator &allocator)
{
    if (curr == NULL) {
        TreeNode *temp = (TreeNode *)allocator.alloc(sizeof(TreeNode));
        temp->key = key;
        // While we do not set the value part here, zero it out
        // to make sure the caller can know that this was a new node.
        memset(&temp->value, 0, sizeof(V));
        temp->height = 0;
        temp->left = NULL;
        temp->right = NULL;
        _num_elems++;
        r = &(temp->value);
        return temp;
    }
    if (key == curr->key) {
        r = &(curr->value);
        return curr;
    }
    else if (key < curr->key) {
        curr->left = add_recursive(curr->left, key, r, allocator);
        if (height(curr->left) - height(curr->right) == 2) { // intolerable imbalance
            if (key > curr->left->key)
                curr = rightleft_rotate(curr);
            else
                curr = right_rotate(curr);
        }
    }
    else {
        curr->right = add_recursive(curr->right, key, r, allocator);
        if (height(curr->left) - height(curr->right) == -2) { // intolerable imbalance
            if (key < curr->right->key)
                curr = leftright_rotate(curr);
            else
                curr = left_rotate(curr);
        }
    }

    curr->height = max(height(curr->left), height(curr->right)) + 1;
    return curr;
}

template <typename K, typename V>
V *AvlTreeIndex<K,V>::add(const K &key, Allocator &allocator)
{
    V *r = NULL;
    _tree = add_recursive(_tree, key, r, allocator);
    return r;
}

template <typename K, typename V>
typename AvlTreeIndex<K,V>::TreeNode *AvlTreeIndex<K,V>::remove_recursive(AvlTreeIndex<K,V>::TreeNode *curr,
                                                const K &key, Allocator &allocator)
{
    if (curr == NULL) {
        return NULL;
    }
    if (key == curr->key) {
        // If no children or just one child, just free given node and return
        // non-empty child or NULL
        if (curr->left == NULL || curr->right == NULL) {
            _num_elems--;
            TreeNode *temp = curr->left != NULL ? curr->left : curr->right;
            allocator.free(curr, sizeof *curr);
            // This nodes and its own childrens' heights won't get affected
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
        curr->key = to_replace->key;
        curr->left = remove_recursive(curr->left, to_replace->key, allocator);
        return curr;
    }
    else if (key < curr->key) {
        curr->left = remove_recursive(curr->left, key, allocator);
        // Right tree too large?
        if (height(curr->left) - height(curr->right) == -2) {
            //if (key < curr->right->key)
            if (curr->right->left != NULL)
                curr = leftright_rotate(curr);
            else
                curr = left_rotate(curr);
        }
    }
    else {
        curr->right = remove_recursive(curr->right, key, allocator);
        // Left tree too large?
        if (height(curr->left) - height(curr->right) == 2) {
            //if (key > curr->left->key)
            if (curr->left->right != NULL)
                curr = rightleft_rotate(curr);
            else
                curr = right_rotate(curr);
        }
    }

    curr->height = max(height(curr->left), height(curr->right)) + 1;
    return curr;
}

template <typename K, typename V>
void AvlTreeIndex<K,V>::remove(const K &key, Allocator &allocator)
{
    _tree = remove_recursive(_tree, key, allocator);
}

template <typename K, typename V>
V *AvlTreeIndex<K,V>::find(const K &key)
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

// Explicitly instantiate any types that might be required
template class AvlTreeIndex<int,int>;
template class AvlTreeIndex<long long,List<Node *>>;
template class AvlTreeIndex<bool,List<Node *>>;
template class AvlTreeIndex<double,List<Node *>>;
