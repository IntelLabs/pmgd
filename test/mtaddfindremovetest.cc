/**
 * @file   mtaddfindremovetest.cc
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

#define __STDC_FORMAT_MACROS
#include <iostream>
#include <inttypes.h>
#include <stdlib.h>
#include <thread>
#include <atomic>
#include <mutex>
#include "pmgd.h"
#include "util.h"

using namespace PMGD;
using namespace std;

namespace PMGD {
    class MTAddFindRemoveTest {
        static const unsigned PEACE_TIME = 10;  // microseconds.
        static const unsigned NUM_INSERT_THREADS = 4;
        static const unsigned NUM_FIND_THREADS = 4;
        static const unsigned NUM_NODES_PER_TX = 35;
        static const unsigned NUM_TX_PER_THREAD = 10;
        static const unsigned MAX_RETRIES = 100000;

        Graph::Config _config;
        Graph _db;

        atomic<long> _nodes_added, _nodes_removed;
        atomic<long> _edges_added, _edges_removed;
        atomic<long> _sum_tag1_id1;
        atomic<long> _sum_id3;
        atomic<int> _num_tag2_id2_str;
        atomic<int> _num_tag2_id2_apple;
        atomic<int> _num_tag2_id4;
        atomic<int> _add_threads_done;
        atomic<int> _read_exceptions;
        atomic<int> _retval;
        atomic<bool> _remove_threads_done[2];

        int add_thread(int tid);
        int find_thread(int tid);
        int read_thread1(int tid);
        int read_thread2(int tid);
        int read_thread3(int tid);
        int read_thread4(int tid);
        int remove_thread1(int tid);
        int remove_thread2(int tid);
        int read_thread5(int tid);
        int read_thread6(int tid);
        int read_thread7(int tid);

    public:
        MTAddFindRemoveTest();
        int run_test();
    };
}

const unsigned PMGD::MTAddFindRemoveTest::PEACE_TIME;

int main()
{
    int ret = 0;
    int repeat = 1;

    for (unsigned i = 0; i < repeat; ++i) {
        cout << i << ": MT-AddFind unit test\n";
        if (system("rm -rf ./mtaddfindremovegraph") < 0)
            exit(-1);
        MTAddFindRemoveTest mtatt;
        ret += mtatt.run_test();
        cout << endl;
    }

    return ret;
}

Graph::Config *init_config(Graph::Config *config)
{
    config->allocator_region_size = 104857600;  // 100MB
    config->num_allocators = 4;
    return config;
}

MTAddFindRemoveTest::MTAddFindRemoveTest()
    : _db("mtaddfindremovegraph", Graph::Create, init_config(&_config))
{
    try {
        Transaction tx(_db, Transaction::ReadWrite);

        _db.create_index(Graph::NodeIndex, "tag1", "id1", PropertyType::Integer);
        _db.create_index(Graph::NodeIndex, "tag1", "id2", PropertyType::Float);
        _db.create_index(Graph::NodeIndex, "tag2", "id1", PropertyType::Float);
        _db.create_index(Graph::NodeIndex, "tag2", "id2", PropertyType::String);
        _db.create_index(Graph::NodeIndex, 0, "id3", PropertyType::Integer);
        _db.create_index(Graph::NodeIndex, "tag2", "id4", PropertyType::Integer);
        
        // To check for a null iterator, we use a non-existent stringid which leads
        // to a new entry in string table. Avoid that.
        StringID random("tag3");
        tx.commit();
    } catch (Exception e) {
        print_exception(e);
        exit(-1);
    }
    _nodes_added = 0;
    _edges_added = 0;
    _sum_tag1_id1 = 0;
    _sum_id3 = 0;
    _num_tag2_id2_str = 0;
    _num_tag2_id2_apple = 0;
    _num_tag2_id4 = 0;
    _add_threads_done = 0;
    _remove_threads_done[0] = false;
    _remove_threads_done[1] = false;
    _read_exceptions = 0;
    _retval = 0;
    _nodes_removed = 0;
    _edges_removed = 0;
}

int MTAddFindRemoveTest::run_test()
{
    vector<thread> threads;

    // *** TODO Read allocator stats at various spots to verify.
    // per alloc thread
    printf("MTAddFindRemoveTest create %d threads\n", NUM_INSERT_THREADS + NUM_FIND_THREADS);
    for (int i = 0; i < NUM_INSERT_THREADS; ++i)
        threads.push_back(thread(&PMGD::MTAddFindRemoveTest::add_thread, this, i));
    for (int i = NUM_INSERT_THREADS; i < NUM_INSERT_THREADS + NUM_FIND_THREADS; ++i)
        threads.push_back(thread(&PMGD::MTAddFindRemoveTest::find_thread, this, i));

    for (auto& th : threads) {
        printf("Joining threads\n");
        th.join();
    }
    threads.clear();
    printf("Finished join\n");
    cout << "Num read exceptions: " <<  _read_exceptions << endl;
    cout << "Total num_tag2_id4: " << _num_tag2_id4 << ", num_apple: " << _num_tag2_id2_apple << endl;
    cout << "Nodes added: " << _nodes_added << ", edges: " << _edges_added << endl;
    _read_exceptions = 0;
    
    printf("Now test remove\n");
    int tid = NUM_INSERT_THREADS + NUM_FIND_THREADS;
    threads.push_back(thread(&PMGD::MTAddFindRemoveTest::remove_thread1, this, tid));
    tid++;
    threads.push_back(thread(&PMGD::MTAddFindRemoveTest::read_thread5, this, tid));
    tid++;
    threads.push_back(thread(&PMGD::MTAddFindRemoveTest::remove_thread2, this, tid));
    tid++;
    threads.push_back(thread(&PMGD::MTAddFindRemoveTest::read_thread6, this, tid));
    tid++;
    threads.push_back(thread(&PMGD::MTAddFindRemoveTest::read_thread7, this, tid));

    for (auto& th : threads) {
        printf("Joining threads\n");
        th.join();
    }
    printf("Finished join\n");
    cout << "Num read exceptions: " <<  _read_exceptions << endl;
    cout << "Nodes removed: " << _nodes_removed << ", edges: " << _edges_removed << endl;
    return _retval;
}

int MTAddFindRemoveTest::add_thread(int tid)
{
    int node_count = NUM_NODES_PER_TX;
    int node_offset = tid * NUM_NODES_PER_TX * NUM_TX_PER_THREAD;
    int edge_count = node_count - 1;
    int nodes_added = 0, edges_added = 0;
    long sum_tag1_id1 = 0, temp_tag1_id1;
    long sum_id3 = 0.0, temp_id3;
    int num_tag2_id2_str = 0, count_tag2_id2_str;
    int num_tag2_id2_apple = 0, count_tag2_id2_apple;
    int num_tag2_id4 = 0, count_tag2_id4;

    printf("Thread id: %d, %s\n", tid, __FUNCTION__);
    for (int j = 0; j < NUM_TX_PER_THREAD; ++j) {
        Node **nodes = new Node *[node_count + 1];
        node_offset += j;
        try {
            Transaction tx(_db, Transaction::ReadWrite);
            temp_tag1_id1 = 0;
            temp_id3 = 0;
            for (int i = 1; i <= 2; i++) {
                Node &n = _db.add_node("tag1");
                n.set_property("id1", i + 1611);
                temp_tag1_id1 += i + 1611;
                n.set_property("id3", i + node_offset + 2000);
                temp_id3 += i + node_offset + 2000;
                nodes[i] = &n;
            }
            for (int i = 3; i <= 4; i++) {
                Node &n = _db.add_node("tag1");
                // More nodes with the same property value and tag
                n.set_property("id1", i - 2 + 1611);
                temp_tag1_id1 += i - 2 + 1611;
                nodes[i] = &n;
            }
            for (int i = 5; i <= 7; i++) {
                Node &n = _db.add_node("tag1");
                n.set_property("id2", i + 23.57);
                nodes[i] = &n;
            }
            count_tag2_id4 = 0;
            for (int i = 8; i <= 8; i++) {
                Node &n = _db.add_node("tag2");
                n.set_property("id1", i + node_offset + 23.57);
                n.set_property("id4", 1000);
                count_tag2_id4++;
                nodes[i] = &n;
            }
            // String property
            count_tag2_id2_str = 0;
            for (int i = 9; i <= 10; i++) {
                Node &n = _db.add_node("tag2");
                n.set_property("id2", "This is string test");
                n.set_property("id3", i + 2000);
                temp_id3 += i + 2000;
                count_tag2_id2_str++;
                nodes[i] = &n;
            }
            for (int i = 11; i <= 11; i++) {
                Node &n = _db.add_node("tag2");
                n.set_property("id2", "This is awesome");
                n.set_property("id3", i + node_offset + 2000);
                temp_id3 += i + node_offset + 2000;
                nodes[i] = &n;
            }
            count_tag2_id2_apple = 0;
            for (int i = 12; i <= 16; i++) {
                Node &n = _db.add_node("tag2");
                n.set_property("id2", "An apple");
                count_tag2_id2_apple++;
                n.set_property("id3", i + node_offset);
                temp_id3 += i + node_offset;
                nodes[i] = &n;
            }
            for (int i = 17; i <= node_count; i++) {
                Node &n = _db.add_node("tag2");
                n.set_property("id2", "An apple and peach.");
                n.set_property("id3", i + node_offset + 4000);
                temp_id3 += i + node_offset + 4000;
                nodes[i] = &n;
            }

            for (int i = 1; i <= edge_count; i++) {
                Edge &e = _db.add_edge(*nodes[i], *nodes[i+1], 0);
                e.set_property("id", i + node_offset + 2611);
            }
            //dump_debug(_db);
            tx.commit();
            nodes_added += node_count;
            edges_added += edge_count;
            sum_tag1_id1 += temp_tag1_id1;
            sum_id3 += temp_id3;
            num_tag2_id2_str += count_tag2_id2_str;  // Matching the string "This is string test"
            num_tag2_id2_apple += count_tag2_id2_apple;  // String == "An apple"
            num_tag2_id4 += count_tag2_id4;  // Int value = 1000
        } catch (Exception e) {
            // printf("Thread %d: \n", tid);
            // print_exception(e);
        }
        this_thread::sleep_for(chrono::microseconds(tid * PEACE_TIME));
    }

    _nodes_added += nodes_added;
    _edges_added += edges_added;
    _sum_tag1_id1 += sum_tag1_id1;
    _sum_id3 += sum_id3;
    _num_tag2_id2_str += num_tag2_id2_str;
    _num_tag2_id2_apple += num_tag2_id2_apple;
    _num_tag2_id4 += num_tag2_id4;  // Int value = 1000
    _add_threads_done++;

    printf("%s: , ", __FUNCTION__);
    cout << "Thread Id: " << tid << ", nodes added: " << nodes_added << ", edges: " << edges_added << endl;

    return 0;
}

int MTAddFindRemoveTest::read_thread1(int tid)
{
    long sum_tag1_id1;
    int retries = 0;

    printf("Thread id: %d, %s\n", tid, __FUNCTION__);

    while(true) {
        if (retries++ > MAX_RETRIES)
            return -1;
        try {
            Transaction tx(_db, Transaction::ReadOnly);
            // printf("## Trying iterator with tag tag1 and property id1\n");
            sum_tag1_id1 = 0;
            for (NodeIterator i = _db.get_nodes("tag1"); i; i.next()) {
                Property p;
                if (i->check_property("id1", p))
                    sum_tag1_id1 += p.int_value();
            }
            if (sum_tag1_id1 == _sum_tag1_id1 && _add_threads_done == NUM_INSERT_THREADS)
                break;
            this_thread::sleep_for(chrono::microseconds(tid * PEACE_TIME));
        }
        catch (Exception e) {
            // printf("Thread id: %d\n", tid);
            _read_exceptions++;
            // print_exception(e);
        }
    }
    printf("%s: Thread %d, Retries %d\n", __FUNCTION__, tid, retries);
    return 0;
}

int MTAddFindRemoveTest::read_thread2(int tid)
{
    long sum_id3;
    int retries = 0;

    printf("Thread id: %d, %s\n", tid, __FUNCTION__);

    while(true) {
        if (retries++ > MAX_RETRIES)
            return -1;
        try {
            Transaction tx(_db, Transaction::ReadOnly);
            // printf("## Trying iterator with tag tag1 and property id2\n");
            sum_id3 = 0;
            PropertyPredicate pp3("id3");
            for (NodeIterator i = _db.get_nodes(0, pp3); i; i.next())
                sum_id3 += i->get_property("id3").int_value();
            if (sum_id3 == _sum_id3 && _add_threads_done == NUM_INSERT_THREADS)
                break;
            this_thread::sleep_for(chrono::microseconds(tid * PEACE_TIME));
        }
        catch (Exception e) {
            // printf("Thread id: %d\n", tid);
            _read_exceptions++;
            // print_exception(e);
        }
    }
    printf("%s: Thread %d, Retries %d\n", __FUNCTION__, tid, retries);
    return 0;
}

int MTAddFindRemoveTest::read_thread3(int tid)
{
    int num_tag2_id2_str;
    int retries = 0;

    printf("Thread id: %d, %s\n", tid, __FUNCTION__);

    while(true) {
        if (retries++ > MAX_RETRIES)
            return -1;
        try {
            Transaction tx(_db, Transaction::ReadOnly);
            // printf("## Trying iterator with tag tag2 and property id2\n");
            num_tag2_id2_str = 0;
            PropertyPredicate pps("id2", PropertyPredicate::Eq, "This is string test");
            for (NodeIterator i = _db.get_nodes("tag2", pps); i; i.next())
                num_tag2_id2_str++;
            if (num_tag2_id2_str == _num_tag2_id2_str && _add_threads_done == NUM_INSERT_THREADS)
                break;
            this_thread::sleep_for(chrono::microseconds(tid * PEACE_TIME));
        }
        catch (Exception e) {
            // printf("Thread id: %d\n", tid);
            _read_exceptions++;
            // print_exception(e);
        }
    }
    printf("%s: Thread %d, Retries %d\n", __FUNCTION__, tid, retries);
    return 0;
}

int MTAddFindRemoveTest::read_thread4(int tid)
{
    int nodes_added = 0, edges_added = 0;
    int retries = 0;

    printf("Thread id: %d, %s\n", tid, __FUNCTION__);

    while(true) {
        if (retries++ > MAX_RETRIES) {
            printf("Thread %d, nodes_added %d, edges_added %d\n", tid, nodes_added, edges_added);
            return -1;
        }
        try {
            Transaction tx(_db, Transaction::ReadOnly);
            // printf("## Trying plain get_nodes() and get_edges() iterator\n");
            nodes_added = edges_added = 0;
            for (NodeIterator i = _db.get_nodes(); i; i.next())
                nodes_added++;
            for (EdgeIterator i = _db.get_edges(); i; i.next())
                edges_added++;
            if (nodes_added == _nodes_added && edges_added == _edges_added
                    && _add_threads_done == NUM_INSERT_THREADS)
                break;
            this_thread::sleep_for(chrono::microseconds(tid * PEACE_TIME));
        }
        catch (Exception e) {
            // printf("Thread id: %d\n", tid);
            _read_exceptions++;
            // print_exception(e);
        }
    }
    printf("%s: Thread %d, Retries %d\n", __FUNCTION__, tid, retries);
    return 0;
}

int MTAddFindRemoveTest::find_thread(int tid)
{
    int retval = 0;
    printf("Thread id: %d, %s\n", tid, __FUNCTION__);
    this_thread::sleep_for(chrono::microseconds(tid * PEACE_TIME));
    switch (tid % NUM_INSERT_THREADS) {
    case 0:
        retval = read_thread1(tid);
        break;
    case 1:
        retval = read_thread2(tid);
        break;
    case 2:
        retval = read_thread3(tid);
        break;
    case 3:
        retval = read_thread4(tid);
        break;
    }
    printf("%s: Thread %d, retval %d\n", __FUNCTION__, tid, retval);
    _retval += retval;
    return retval;
}

int MTAddFindRemoveTest::remove_thread1(int tid)
{
    int retval = 0;
    int removed_num_tag2_id2_apple;
    int retries = 0;
    printf("Thread id: %d, %s\n", tid, __FUNCTION__);
    this_thread::sleep_for(chrono::microseconds(tid * PEACE_TIME));

    while (true) {
        if (retries++ > MAX_RETRIES) {
            retval = -1;
            break;
        }
        try {
            removed_num_tag2_id2_apple = 0;
            Transaction tx(_db, Transaction::ReadWrite);
            PropertyPredicate pp("id2", PropertyPredicate::Eq, "An apple");
            for (NodeIterator ni = _db.get_nodes("tag2", pp); ni; ni.next()) {
                _db.remove(*ni);
                removed_num_tag2_id2_apple++;
            }
            _remove_threads_done[0] = true;  // to make sure the read thread knows we are done on time
            tx.commit();
            printf("Removed tag2_id2_apple %d\n", removed_num_tag2_id2_apple);
            // This has to match all together since remove threads are created after
            // add threads are all done.
            if (_num_tag2_id2_apple == removed_num_tag2_id2_apple) {
                _nodes_removed += removed_num_tag2_id2_apple;
                break;
            }
        } catch (Exception e) {
             _remove_threads_done[0] = false;
             printf("Thread id: %d\n", tid);
             print_exception(e);
        }
    }
    printf("%s: Thread %d, Retries %d, retval %d\n", __FUNCTION__, tid, retries, retval);
    _remove_threads_done[0] = true;
    _retval += retval;
    return retval;
}

int MTAddFindRemoveTest::read_thread5(int tid)
{
    int num_tag2_id2_apple;
    int retries = 0;
    int retval = 0;

    printf("Thread id: %d, %s\n", tid, __FUNCTION__);

    while(true) {
        if (retries++ > MAX_RETRIES) {
            retval = -1;
            break;
        }
        try {
            num_tag2_id2_apple = 0;
            Transaction tx(_db, Transaction::ReadOnly);
            // Check for nodes meeting opposite condition now
            PropertyPredicate pp("id2", PropertyPredicate::Eq, "An apple");
            for (NodeIterator i = _db.get_nodes("tag2", pp); i; i.next())
                num_tag2_id2_apple++;
            if (num_tag2_id2_apple == 0) {
                if (!_remove_threads_done[0]) {
                    printf("Somehow the apple node count is 0 before remove is done\n");
                    retval = -1;
                }
                break;
            }
            this_thread::sleep_for(chrono::microseconds(tid * PEACE_TIME));
        }
        catch (Exception e) {
            // printf("Thread id: %d\n", tid);
            _read_exceptions++;
            // print_exception(e);
        }
    }
    _retval += retval;
    printf("%s: Thread %d, Retries %d, retval %d\n", __FUNCTION__, tid, retries, retval);
    return 0;
}

int MTAddFindRemoveTest::remove_thread2(int tid)
{
    int retval = 0;
    int removed_num_tag2_id4;
    int retries = 0;
    printf("Thread id: %d, %s\n", tid, __FUNCTION__);
    this_thread::sleep_for(chrono::microseconds(tid * PEACE_TIME));

    while (true) {
        if (retries++ > MAX_RETRIES) {
            retval = -1;
            break;
        }
        try {
            removed_num_tag2_id4 = 0;
            Transaction tx(_db, Transaction::ReadWrite);
            PropertyPredicate pp("id4", PropertyPredicate::Eq, 1000);
            for (NodeIterator ni = _db.get_nodes("tag2", pp); ni; ni.next()) {
                _db.remove(*ni);
                removed_num_tag2_id4++;
            }
            _remove_threads_done[1] = true;  // to make sure the read thread knows we are done on time
            tx.commit();

            // This has to match all together since remove threads are created after
            // add threads are all done.
            if (_num_tag2_id4 == removed_num_tag2_id4) {
                _nodes_removed += removed_num_tag2_id4;
                break;
            }
        } catch (Exception e) {
            _remove_threads_done[1] = false;  // to make sure the read thread knows we are done on time
            // printf("Thread id: %d\n", tid);
            // print_exception(e);
        }
    }
    printf("%s: Thread %d, Retries %d, retval %d\n", __FUNCTION__, tid, retries, retval);
    _remove_threads_done[1] = true;
    _retval += retval;
    return retval;
}

int MTAddFindRemoveTest::read_thread6(int tid)
{
    int num_tag2_id4;
    int retries = 0;
    int retval = 0;

    printf("Thread id: %d, %s\n", tid, __FUNCTION__);

    while(true) {
        if (retries++ > MAX_RETRIES) {
            retval = -1;
            break;
        }
        try {
            Transaction tx(_db, Transaction::ReadOnly);
            num_tag2_id4 = 0;
            PropertyPredicate pp("id4", PropertyPredicate::Eq, 1000);
            for (NodeIterator i = _db.get_nodes("tag2", pp); i; i.next())
                num_tag2_id4++;
            if (num_tag2_id4 == 0) {
                if (!_remove_threads_done[1]) {
                    printf("Somehow the id4 node count is 0 before remove is done\n");
                    retval = -1;
                }
                break;
            }
            this_thread::sleep_for(chrono::microseconds(tid * PEACE_TIME));
        }
        catch (Exception e) {
            // printf("Thread id: %d\n", tid);
            _read_exceptions++;
            // print_exception(e);
        }
    }
    printf("%s: Thread %d, Retries %d, retval %d\n", __FUNCTION__, tid, retries, retval);
    _retval += retval;
    return 0;
}

int MTAddFindRemoveTest::read_thread7(int tid)
{
    int edges = 0;
    int retries = 0;
    int retval = 0;

    printf("Thread id: %d, %s\n", tid, __FUNCTION__);

    while(true) {
        if (retries++ > MAX_RETRIES) {
            retval = -1;
            break;
        }
        try {
            Transaction tx(_db, Transaction::ReadOnly);
            edges = 0;
            if (_remove_threads_done[0] && _remove_threads_done[1]) {
                for (EdgeIterator i = _db.get_edges(); i; i.next())
                    edges++;
                break;
            }
            this_thread::sleep_for(chrono::microseconds(tid * PEACE_TIME));
        }
        catch (Exception e) {
            // printf("Thread id: %d\n", tid);
            _read_exceptions++;
            // print_exception(e);
        }
    }
    _edges_removed = _edges_added - edges;
    printf("%s: Thread %d, Retries %d, retval %d\n", __FUNCTION__, tid, retries, retval);
    _retval += retval;
    return 0;
}

