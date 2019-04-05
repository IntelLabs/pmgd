/**
 * @file   mtavltest.cc
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
#include <vector>
#include <unordered_set>
#include <climits>
#include <atomic>
#include <stdlib.h>
#include <string.h>

#include "pmgd.h"
#include "../src/Allocator.h"
#include "../src/AvlTree.h"
#include "../src/GraphConfig.h"
#include "util.h"

using namespace PMGD;
using namespace std;

namespace PMGD {
    class MTAvlTreeTest {
        static const unsigned PEACE_TIME = 10;  // microseconds.
        static const unsigned NUM_INSERT_THREADS = 5;
        static const unsigned NUM_REMOVE_THREADS = 2;
        static const unsigned NUM_LOOPS = 50;
        static const unsigned NUM_INSERTS = 20;

        Graph::Config _config;
        Graph _db;
        Allocator *_allocator;
        AvlTree<int,int> _tree;

        atomic<long> _add_retries;
        atomic<long> _add_failures;
        atomic<long> _rem_retries;
        atomic<long> _rem_failures;

        vector<unordered_set<int>> myvals;
        int add_thread(int tid);
        int remove_thread(int tid, int ttid);

        bool find_if_inserted(int elem);
        bool find_if_removed(int elem);
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
        MTAvlTreeTest();
        int run_test();
    };
}

const unsigned PMGD::MTAvlTreeTest::PEACE_TIME;

int main()
{
    int ret = 0;
    int repeat = 30; //100

    for (unsigned i = 0; i < repeat; ++i) {
        cout << i << ": MT-AVLTree unit test\n";
        if (system("rm -rf ./mtavlgraph") < 0)
            exit(-1);
        MTAvlTreeTest mtatt;
        ret += mtatt.run_test();
        cout << endl;
    }

    return ret;
}

Graph::Config *init_config(Graph::Config *config)
{
    config->allocator_region_size = 104857600;  // 100MB
    config->num_allocators = std::thread::hardware_concurrency();
    return config;
}

MTAvlTreeTest::MTAvlTreeTest()
    : _db("mtavlgraph", Graph::Create, init_config(&_config)),
      _allocator(Allocator::get_main_allocator(_db)),
      myvals(NUM_INSERT_THREADS + NUM_REMOVE_THREADS)
{
    _add_retries = _add_failures = 0;
    _rem_retries = _rem_failures = 0;
}

int MTAvlTreeTest::run_test()
{
    vector<thread> threads;

    // *** TODO Read allocator stats at various spots to verify.
    // per alloc thread
    printf("MTAvlTest create 5 threads\n");
    for (int i = 0; i < NUM_INSERT_THREADS; ++i) {
        this_thread::sleep_for(chrono::microseconds(i * PEACE_TIME));
        threads.push_back(thread(&PMGD::MTAvlTreeTest::add_thread, this, i));
    }
    for (int i = NUM_INSERT_THREADS; i < NUM_INSERT_THREADS + NUM_REMOVE_THREADS; ++i) {
        this_thread::sleep_for(chrono::microseconds(i * PEACE_TIME));
        threads.push_back(thread(&PMGD::MTAvlTreeTest::remove_thread, this, i, (i - NUM_INSERT_THREADS)));
    }

    for (auto& th : threads) {
        printf("Joining threads\n");
        th.join();
    }
    printf("Finished join\n");
    size_t num_inserted = 0;
    for (int i = 0; i < NUM_INSERT_THREADS; ++i)
        num_inserted += myvals[i].size();
    printf("Inserted %ld/%d\n", num_inserted, NUM_LOOPS * NUM_INSERTS * NUM_INSERT_THREADS);
    size_t num_deleted = 0;
    for (int i = NUM_INSERT_THREADS; i < NUM_INSERT_THREADS + NUM_REMOVE_THREADS; ++i)
        num_deleted += myvals[i].size();
    printf("Deleted %ld/%d\n", num_deleted, NUM_LOOPS * NUM_INSERTS * NUM_REMOVE_THREADS / 4);
    cout << "Total add_retries: " << _add_retries << ", add_failures: " << _add_failures << endl;
    cout << "Total rem_retries: " << _rem_retries << ", rem_failures: " << _rem_failures << endl;
    printf("Max height of tree: %d\n", _tree.height(_tree._tree));

    printf("Let's check the tree\n");
    return check_tree();
}

bool MTAvlTreeTest::find_if_inserted(int elem)
{
    bool found = false;
    for (int i = 0; i < NUM_INSERT_THREADS; ++i) {
        if (myvals[i].find(elem) != myvals[i].end()) {
            found = true;
            break;
        }
    }
    return found;
}

bool MTAvlTreeTest::find_if_removed(int elem)
{
    bool removed = false;
    for (int i = NUM_INSERT_THREADS; i < NUM_REMOVE_THREADS; ++i) {
        if (myvals[i].find(elem) != myvals[i].end()) {
            removed = true;
            break;
        }
    }
    return removed;
}

int MTAvlTreeTest::check_tree_recursive(AvlTree<int,int> &tree, AvlTree<int,int>::TreeNode *node, int level)
{
    if (node == NULL)
        return 0;

    int retval = 0;
    AvlTree<int,int>::TreeNode *nodel, *noder;
    nodel = tree.left(node);
    noder = tree.right(node);
    int rootkey = *tree.key(node);
    if (nodel == node || noder == node) {
        printf("How can left/right child be equal to root? at key: %d\n", rootkey);
        exit(1);
    }

    if (!find_if_inserted(rootkey) && find_if_removed(rootkey)) {
        printf("Key %d should not exist in tree\n", rootkey);
        retval++;
    }
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

int MTAvlTreeTest::check_tree()
{
    return check_tree_recursive(_tree, _tree._tree, 0);
}

int MTAvlTreeTest::add_thread(int tid)
{
    int num_loops = NUM_LOOPS, num_adds = NUM_INSERTS;
    int retry = 0;

    printf("Launching add_thread %d\n", tid);

    for (int i = 0; i < num_loops; ++i) {
        try {
            //printf("...Thread %d, i %d, first try\n", tid, i);
            Transaction tx(_db, Transaction::ReadWrite);
            for (int j = 0; j < num_adds; ++j) {
                int key = tid + (i * num_adds + j) * NUM_INSERT_THREADS;
                int *value = _tree.add(key, *_allocator);
                *value = i + j;
            }
            tx.commit();

            // Add to the data structure only if successful.
            for (int j = 0; j < num_adds; ++j)
                myvals[tid].insert(tid + (i * num_adds + j) * NUM_INSERT_THREADS);
        }
        catch (Exception e) {
            if (e.num != LockTimeout) {
                print_exception(e);
                throw e;
            }
            this_thread::sleep_for(chrono::microseconds(PEACE_TIME));
            //printf("...Thread %d, i %d, retry\n", tid, i);
            try {
                retry++;
                Transaction tx(_db, Transaction::ReadWrite);
                for (int j = 0; j < num_adds; ++j) {
                    int key = tid + (i * num_adds + j) * NUM_INSERT_THREADS;
                    int *value = _tree.add(key, *_allocator);
                    *value = i + j;
                }
                tx.commit();
                //printf("...Thread %d, i %d, inserted %d\n", tid, i, num_adds);

                // Add to the data structure only if successful.
                for (int j = 0; j < num_adds; ++j)
                    myvals[tid].insert(tid + (i * num_adds + j) * NUM_INSERT_THREADS);
            }
            catch (Exception e1) {
                //printf("Failed again\n");
                _add_failures++;
                if (e.num != LockTimeout) {
                    print_exception(e);
                    throw e;
                }
            }
        }
    }

    printf("Thread %d, add_retries %d\n", tid, retry);
    _add_retries += retry;

    return 0;
}

int MTAvlTreeTest::remove_thread(int tid, int ttid)
{
    int num_loops = NUM_LOOPS / 2, num_rems = NUM_INSERTS;
    int retry = 0;
    unordered_set<int> deleted;

    printf("Launching delete_thread %d, targeting %d\n", tid, ttid);

    for (int i = 0; i < num_loops; ++i) {
        try {
            //printf("...Thread %d, i %d, first try\n", tid, i);
            Transaction tx(_db, Transaction::ReadWrite);
            for (int j = 0; j < num_rems; j += 2) {
                int key = ttid + (i * num_rems + j) * NUM_INSERT_THREADS;
                if (_tree.remove(key, *_allocator) == 0)
                    deleted.insert(key);
            }
            tx.commit();

            // Add for checking from the data structure only if successful.
            for (auto it = deleted.begin(); it != deleted.end(); ++it)
                myvals[tid].insert(*it);
        }
        catch (Exception e) {
            if (e.num != LockTimeout) {
                print_exception(e);
                throw e;
            }
            this_thread::sleep_for(chrono::microseconds(2 * PEACE_TIME));
            //printf("...Thread %d, i %d, retry\n", tid, i);
            try {
                retry++;
                Transaction tx(_db, Transaction::ReadWrite);
                for (int j = 0; j < num_rems; j += 2) {
                    int key = ttid + (i * num_rems + j) * NUM_INSERT_THREADS;
                    if (_tree.remove(key, *_allocator) == 0)
                        deleted.insert(key);
                }
                tx.commit();
                //printf("...Thread %d, i %d, inserted %d\n", tid, i, num_adds);

                // Add for checking from the data structure only if successful.
                for (auto it = deleted.begin(); it != deleted.end(); ++it)
                    myvals[tid].insert(*it);
            }
            catch (Exception e1) {
                //printf("Failed again\n");
                _rem_failures++;
                if (e.num != LockTimeout) {
                    print_exception(e);
                    throw e;
                }
            }
        }
    }

    printf("Thread %d, rem_retries %d\n", tid, retry);
    _rem_retries += retry;

    return 0;
}

