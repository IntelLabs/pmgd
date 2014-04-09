#include "AvlTree.h"

using namespace Jarvis;

// hinge-->new_root-->its left and right children
// The return value of this function gets assigned to
// the parent of hinge that will call this function
// Also called rotate_with_right
template <typename T>
typename AvlTree<T>::AvlTreeNode *AvlTree<T>::left_rotate(AvlTree<T>::AvlTreeNode *hinge)
{
    AvlTreeNode *new_root = hinge->right;
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
template <typename T>
typename AvlTree<T>::AvlTreeNode *AvlTree<T>::right_rotate(AvlTree<T>::AvlTreeNode *hinge)
{
    AvlTreeNode *new_root = hinge->left;
    hinge->left = new_root->right;
    hinge->height = max(height(hinge->left), height(hinge->right)) + 1;
    new_root->right = hinge;
    // TODO this might be a repeat given what follows in the caller
    // but for ensuring correct heights, lets set this anyway.
    new_root->height = max(height(hinge), height(new_root->left)) + 1;
    return new_root;
}

// a
//  \
//   c
//  /
// b
// Also called double left
template <typename T>
typename AvlTree<T>::AvlTreeNode *AvlTree<T>::leftright_rotate(AvlTree<T>::AvlTreeNode *hinge)
{
    hinge->right = right_rotate(hinge->right);
    return left_rotate(hinge);
}

//   c
//  /
// a
//  \
//   b
// Also called double right  
template <typename T>
typename AvlTree<T>::AvlTreeNode *AvlTree<T>::rightleft_rotate(AvlTree<T>::AvlTreeNode *hinge)
{
    hinge->left = left_rotate(hinge->left);
    return right_rotate(hinge);
}

template <typename T>
typename AvlTree<T>::AvlTreeNode *AvlTree<T>::add_recursive(AvlTree<T>::AvlTreeNode *curr,
                                                T &data, Allocator &allocator)
{
    if (curr == NULL) {
        AvlTreeNode *temp = (AvlTreeNode *)allocator.alloc(sizeof(AvlTreeNode));
        temp->data = data;
        temp->height = 0;
        temp->left = NULL;
        temp->right = NULL;
        _num_elems++;
        return temp;
    }
    if (data == curr->data) {
        return curr;
    }
    else if (data < curr->data) {
        curr->left = add_recursive(curr->left, data, allocator);
        if (height(curr->left) - height(curr->right) == 2) { // intolerable imbalance
            if (data > curr->left->data)
                curr = rightleft_rotate(curr);
            else
                curr = right_rotate(curr);
        }
    }
    else {
        curr->right = add_recursive(curr->right, data, allocator);
        if (height(curr->left) - height(curr->right) == -2) { // intolerable imbalance
            if (data < curr->right->data)
                curr = leftright_rotate(curr);
            else
                curr = left_rotate(curr);
        }
    }

    curr->height = max(height(curr->left), height(curr->right)) + 1;
    return curr;
}

template <typename T>
void AvlTree<T>::add(T &data, Allocator &allocator)
{
    _tree = add_recursive(_tree, data, allocator);
}

template <typename T>
typename AvlTree<T>::AvlTreeNode *AvlTree<T>::remove_recursive(AvlTree<T>::AvlTreeNode *curr,
                                                T &data, Allocator &allocator)
{
    if (curr == NULL) {
        return NULL;
    }
    if (data == curr->data) {
        // If no children, just free and return
        if (curr->left == NULL || curr->right == NULL) {
            _num_elems--;
            AvlTreeNode *temp = curr->left != NULL ? curr->left : curr->right;
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
        AvlTreeNode *to_replace = find_max(curr->left);
        // This node will never be NULL.
        curr->data = to_replace->data;
        curr->left = remove_recursive(curr->left, to_replace->data, allocator);
        return curr;
    }
    else if (data < curr->data) {
        curr->left = remove_recursive(curr->left, data, allocator);
        // Right tree too large?
        if (height(curr->left) - height(curr->right) == -2) {
            //if (data < curr->right->data)
            if (curr->right->left != NULL)
                curr = leftright_rotate(curr);
            else
                curr = left_rotate(curr);
        }
    }
    else {
        curr->right = remove_recursive(curr->right, data, allocator);
        // Left tree too large?
        if (height(curr->left) - height(curr->right) == 2) {
            //if (data > curr->left->data)
            if (curr->left->right != NULL)
                curr = rightleft_rotate(curr);
            else
                curr = right_rotate(curr);
        }
    }

    curr->height = max(height(curr->left), height(curr->right)) + 1;
    return curr;
}

template <typename T>
void AvlTree<T>::remove(T &data, Allocator &allocator)
{
    _tree = remove_recursive(_tree, data, allocator);
}

template <typename T>
typename AvlTree<T>::AvlTreeNode *AvlTree<T>::find(T &data)
{
    AvlTreeNode *curr = _tree;
    while (curr != NULL) {
        if (data == curr->data)
            return curr;
        else if (data < curr->data)
            curr = curr->left;
        else
            curr = curr->right;
    }
    return NULL;
}

template <typename T>
void AvlTree<T>::add_left_tree(std::set<AvlTree<T>::AvlTreeNode *> &range,
                            AvlTree<T>::AvlTreeNode *curr,
                            T &min, bool inclusive)
{
    if (curr == NULL)
        return;
    // Right tree is explored in all these options
    add_left_tree(range, curr->right, min, inclusive);
    if (curr->data > min) {
        range.insert(curr);
        add_left_tree(range, curr->left, min, inclusive);
    }
    else if (curr->data == min) {
        if (inclusive)
            range.insert(curr);
        // No left tree anymore since all elements will be lower
    }
}

template <typename T>
void AvlTree<T>::add_right_tree(std::set<AvlTree<T>::AvlTreeNode *> &range,
                             AvlTree<T>::AvlTreeNode *curr,
                             T &max, bool inclusive, bool *done)
{
    if (curr == NULL)
        return;
    // Left tree is explored in all these options
    add_right_tree(range, curr->left, max, inclusive, done);
    if (curr->data < max) {
        range.insert(curr);
        add_right_tree(range, curr->right, max, inclusive, done);
    }
    else if (curr->data == max) {
        if (inclusive)
            range.insert(curr);
        *done = true;  // We found max as well
        // No right tree anymore, all elements will be greater
    }
    else // Current node is already large, but have to look at the left tree
        *done = true;
}

template <typename T>
void AvlTree<T>::find_range(std::set<AvlTree<T>::AvlTreeNode *> &r, T &min, T &max, bool inclusive)
{
    bool done = false;
    AvlTreeNode *curr = _tree;
    // First get on the way to min, then to max
    while (curr != NULL) {
        if (curr->data == min) {
            if (inclusive)
                r.insert(curr);
            add_right_tree(r, curr->right, max, inclusive, &done); 
            break;  // since the next element will be smaller
        }
        else if (curr->data < min)
            curr = curr->right;
        else {  // curr data is greater than min, so it belongs if > max
            if (curr->data == max) {
                if (inclusive)
                    r.insert(curr);
            }
            else if (curr->data < max) {
                r.insert(curr);
                add_right_tree(r, curr->right, max, inclusive, &done); 
            }
            curr = curr->left;
        }
    }

    // Now we have to look for max
    if (!done) {
        curr = _tree;
        while (curr != NULL) {
            if (curr->data == max) {
                if (inclusive)
                    r.insert(curr);
                add_left_tree(r, curr->left, min, inclusive); 
                break;  // since the next element will be larger
            }
            else if (curr->data > max)
                curr = curr->left;
            else {  // curr data is smaller than max, so it belongs
                r.insert(curr);
                add_left_tree(r, curr->right, max, inclusive); 
                curr = curr->right;
            }
        }
    }
}

// Explicitly instantiate any types that might be required
template class AvlTree<int>;
