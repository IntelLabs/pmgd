/**
 * @file   avltest.cc
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

#include <iostream>
#include <climits>
#include <string.h>

#include "pmgd.h"
#include "../src/List.h"
#include "../src/os.h"
#include "../src/Allocator.h"
#include "../src/AvlTree.h"
#include "../src/GraphConfig.h"
#include "util.h"

using namespace PMGD;
using namespace std;

namespace PMGD {
    class AvlTreeTest {
        AvlTree<int,int> _tree;
        int run_tree_test(Graph &db, Allocator &allocator1);
        int run_remove_test1(Graph &db, Allocator &allocator1);
        int run_remove_test2(Graph &db, Allocator &allocator1);
        int run_abort_test(Graph &db, Allocator &allocator1);
        int check_tree_recursive(AvlTree<int,int> &tree, AvlTree<int,int>::TreeNode *node, int level);
        int check_tree();

        void print_recursive(AvlTree<int,int> &tree, AvlTree<int,int>::TreeNode *node,
                char side, int rec_depth)
        {
            if (node == NULL)
                return;
            for (int i = 0; i < rec_depth; ++i)
                cout << " ";
            cout << side << ":" << *tree.key(node) << "," << *tree.value(node) << " [" << tree.height(node) << "]\n";
            print_recursive(tree, tree.left(node), 'L', rec_depth + 1);
            print_recursive(tree, tree.right(node), 'R', rec_depth + 1);
        }

        void print()
        {
            print_recursive(_tree, _tree._tree, 'C', 0);
            cout << endl;
        }

    public:
        int run_test();
    };
}

int main()
{
    cout << "AVLTree unit test\n\n";
    AvlTreeTest att;
    return att.run_test();
}

int AvlTreeTest::check_tree_recursive(AvlTree<int,int> &tree, AvlTree<int,int>::TreeNode *node, int level)
{
    if (node == NULL)
        return 0;

    int retval = 0;
    AvlTree<int,int>::TreeNode *nodel, *noder;
    int rootkey = *tree.key(node);
    nodel = tree.left(node);
    noder = tree.right(node);

    int hl = tree.height(nodel);
    int hr = tree.height(noder);
    int diff = hl - hr;
    if (diff <= -2 || diff >= 2) {
        printf("AVL condition messed up at key %d, hleft: %d, hright: %d, level: %d\n", rootkey, hl, hr, level);
        retval++;
    }
    int keyl = -1, keyr = INT_MAX;
    if (nodel != NULL)
        keyl = *tree.key(nodel);
    if (noder != NULL)
        keyr = *tree.key(noder);
    if (keyl >= rootkey || keyr <= rootkey) {
        printf("Value ordered messed up at level %d: L %d, C %d, R %d\n", level, keyl, rootkey, keyr);
        retval++;
    }

    retval += check_tree_recursive(tree, nodel, (level + 1));
    retval += check_tree_recursive(tree, noder, (level + 1));

    return retval;
}

int AvlTreeTest::check_tree()
{
    return check_tree_recursive(_tree, _tree._tree, 0);
}

int AvlTreeTest::run_tree_test(Graph &db, Allocator &allocator1)
{
    Transaction tx(db, Transaction::ReadWrite);
    int insert_vals[] = {5, 5, 10, 15, 20, 25, 1, 4, 30, 35, 40, 45, 50,
        12, 17, 60, 70, 70, 55, 19};
    int num_inserted = 18 + 2;

    for (int i = 0; i < num_inserted; ++i) {
        int *value = _tree.add(insert_vals[i], allocator1);
        *value = i + 1;
    }
    cout << "Num elements: " << _tree.num_elems() << "\n";
    print();

    int find_val = 66;
    int *value = _tree.find(find_val);
    cout << "66: " << ((value == NULL) ? 0 : *value) << endl;
    find_val = 50;
    value = _tree.find(find_val);
    cout << "50: " << ((value == NULL) ? 0 : *value) << endl;

    cout << "Testing remove\n";
    int remove_vals[] = {10, 55, 55, 60, 45, 40, 15, 17, 30};
    int num_removed = 9;
    for (int i = 0; i < num_removed; ++i)
        _tree.remove(remove_vals[i], allocator1);
    cout << "Num elements: " << _tree.num_elems() << "\n";
    print();
    // Now just delete the remaining
    int array[] = {5, 20, 25, 12, 19, 1, 4, 35, 50, 70};
    for (int i = 0; i < 10; ++i)
        _tree.remove(array[i], allocator1);
    cout << "Num elements: " << _tree.num_elems() << "\n";
    print();
    return 0;

    for (int i = 0; i < 8; ++i) {
        value = _tree.add(201 + i, allocator1);
        *value = i + 1;
    }
    value = _tree.add(210, allocator1);
    *value = 1;
    value = _tree.add(212, allocator1);
    *value = 1;
    value = _tree.add(200, allocator1);
    *value = 1;
    cout << "Num elements: " << _tree.num_elems() << "\n";
    print();
    for (int i = 0; i < 8; ++i)
        _tree.remove(201 + i, allocator1);
    _tree.remove(212, allocator1);
    _tree.remove(210, allocator1);
    _tree.remove(200, allocator1);
    cout << "Num elements: " << _tree.num_elems() << "\n";
    tx.commit();
    print();

    return check_tree();
}

int AvlTreeTest::run_remove_test1(Graph &db, Allocator &allocator1)
{
    Transaction tx(db, Transaction::ReadWrite);
    int insert_vals[] = {50, 25, 75, 10, 30, 60, 80, 5, 15, 27, 55, 1};
    int num_inserted = 12;

    for (int i = 0; i < num_inserted; ++i) {
        int *value = _tree.add(insert_vals[i], allocator1);
        *value = i + 1;
    }
    cout << "Num elements: " << _tree.num_elems() << "\n";
    print();
    cout << "Removing 80\n";
    _tree.remove(80, allocator1);
    tx.commit();
    print();

    return check_tree();
}

int AvlTreeTest::run_remove_test2(Graph &db, Allocator &allocator1)
{
    Transaction tx(db, Transaction::ReadWrite);
    int insert_vals[] = {100, 50, 150, 25, 75, 115, 175, 10, 30, 60, 80, 110, 120, 160, 190, 5, 15, 27, 55, 105, 112, 1};
    int num_inserted = 22;

    for (int i = 0; i < num_inserted; ++i) {
        int *value = _tree.add(insert_vals[i], allocator1);
        *value = i + 1;
    }
    cout << "Num elements: " << _tree.num_elems() << "\n";
    print();
    cout << "Removing 27\n";
    _tree.remove(27, allocator1);
    cout << "Removing 80\n";
    _tree.remove(80, allocator1);
    cout << "Removing 10\n";
    _tree.remove(10, allocator1);
    cout << "Removing 5\n";
    _tree.remove(5, allocator1);
    cout << "Removing 100\n";
    _tree.remove(100, allocator1);
    cout << "Removing 120\n";
    _tree.remove(120, allocator1);
    cout << "Removing 112\n";
    _tree.remove(112, allocator1);
    tx.commit();
    print();

    return check_tree();
}

int AvlTreeTest::run_abort_test(Graph &db, Allocator &allocator1)
{
    int insert_vals[] = {100, 50, 150, 25, 75, 115, 175, 10, 30, 60, 80, 110, 120, 160, 190, 5, 15, 27, 55, 105, 112, 1};
    int num_inserted = 22, retval = 0;

    {
        Transaction tx(db, Transaction::ReadWrite);
        for (int i = 0; i < num_inserted; ++i) {
            int *value = _tree.add(insert_vals[i], allocator1);
            *value = i + 1;
        }
    }
    {
        Transaction tx(db);
        if (_tree.num_elems() != 0) {
            printf("Tree not null despite abort: %ld!!!\n", _tree.num_elems());
            print();
            retval++;
        }
    }
    {
        Transaction tx(db, Transaction::ReadWrite);
        for (int i = 0; i < num_inserted; ++i) {
            int *value = _tree.add(insert_vals[i], allocator1);
            *value = i + 1;
        }
        tx.commit();
    }
    {
        Transaction tx(db);
        if (_tree.num_elems() != num_inserted) {
            printf("Tree has wrong number of elems despite commit: %ld!!!\n", _tree.num_elems());
            print();
            retval++;
        }
    }
    {
        Transaction tx(db, Transaction::ReadWrite);
        for (int i = 0; i < num_inserted; ++i)
            _tree.remove(insert_vals[i], allocator1);
    }
    {
        Transaction tx(db);
        if (_tree.num_elems() != num_inserted) {
            printf("Tree has wrong number of elems despite abort of delete: %ld!!!\n", _tree.num_elems());
            print();
            retval++;
        }
    }
    {
        Transaction tx(db, Transaction::ReadWrite);
        for (int i = 0; i < num_inserted; ++i)
            _tree.remove(insert_vals[i], allocator1);
        tx.commit();
    }
    {
        Transaction tx(db);
        if (_tree.num_elems() != 0) {
            printf("Tree no empty despite commit of delete: %ld!!!\n", _tree.num_elems());
            print();
            retval++;
        }
    }

    return retval + check_tree();
}

int AvlTreeTest::run_test()
{
    int retval = 0;

    try {
        Graph db("avlgraph", Graph::Create);

        Allocator *allocator1 = Allocator::get_main_allocator(db);

        printf("Commit abort test\n");
        retval += run_abort_test(db, *allocator1);

        printf("Insert test\n");
        retval += run_tree_test(db, *allocator1);

        printf("\nSimple remove test\n");
        retval += run_remove_test1(db, *allocator1);

        printf("\nLess simple remove test\n");
        retval += run_remove_test2(db, *allocator1);
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }
    return retval;
}
