/**
 * @file   mtalloctest.cc
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

#include <stdlib.h>  // for rand
#include <iostream>
#include <string.h>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_map>

#include "pmgd.h"
#include "../src/os.h"
#include "../src/Allocator.h"
#include "../src/GraphConfig.h"
#include "util.h"

using namespace PMGD;
using namespace std;

namespace PMGD {
    class MTAllocTest {
        static const unsigned PEACE_TIME = 100000;  // microseconds.
        static const unsigned NUM_ALLOC_THREADS = 4;
        static const unsigned NUM_FREE_THREADS = 2;
        static const unsigned NUM_ALLOC_FREE_THREADS = 2;

        Graph::Config _config;
        Graph _db;
        Allocator *_allocator;

        // Save all the addresses from alloc_thread to be freed
        // in free thread
        vector<list<pair<void *, size_t>> > _tofree;
        vector<mutex> _freelocks;
        vector<bool> _allocdone;

        // Error across threads.
        int _error;

    public:
        MTAllocTest(int num_allocators);
        int alloc_thread(int tid);
        int free_thread(int tid);
        int alloc_free_thread(int tid);
        int run_test();
    };
}

// For debug build
const unsigned PMGD::MTAllocTest::PEACE_TIME;

int main()
{
    cout << "MTAlloc unit test with 5 allocators\n\n";
    cout << std::thread::hardware_concurrency() << endl;
    MTAllocTest mtat(std::thread::hardware_concurrency());
    int r = mtat.run_test();
    printf("%d\n", r);

    return r;
}

Graph::Config *init_config(Graph::Config *config, int num_allocators)
{
    config->allocator_region_size = 1048576000;  // 1000MB
    config->num_allocators = num_allocators;
    return config;
}

MTAllocTest::MTAllocTest(int num_allocators)
    : _db("mtallocgraph", Graph::Create, init_config(&_config, num_allocators)),
    _allocator(Allocator::get_main_allocator(_db)),
    _tofree(NUM_ALLOC_THREADS),
    _freelocks(NUM_ALLOC_THREADS),
    _allocdone(NUM_ALLOC_THREADS, false)
  {}

int MTAllocTest::run_test()
{
    vector<thread> threads;
    unsigned tcount;
    _error = 0;

    // per alloc thread
    printf("MTAllocTest create 6 threads\n");
    for (int i = 0; i < NUM_ALLOC_THREADS; ++i)
        threads.push_back(thread(&PMGD::MTAllocTest::alloc_thread, this, i));
    tcount = NUM_ALLOC_THREADS;
    for (int i = tcount; i < NUM_ALLOC_THREADS + NUM_FREE_THREADS; ++i)
        threads.push_back(thread(&PMGD::MTAllocTest::free_thread, this, i));
    tcount += NUM_FREE_THREADS;
    for (int i = tcount; i < tcount + NUM_ALLOC_FREE_THREADS; ++i)
        threads.push_back(thread(&PMGD::MTAllocTest::alloc_free_thread, this, i));
    tcount += NUM_ALLOC_FREE_THREADS;

    for (auto& th : threads)
        th.join();
    printf("Finished join\n");

    for (int i = 0; i < NUM_ALLOC_THREADS ; ++i) {
        if (!_tofree[i].empty()) {
            printf("Thread %d: %ld elements not freed\n", i, _tofree[i].size());
            _error++;
        }
    }

    return _error;
}

int MTAllocTest::alloc_thread(int tid)
{
    unsigned fixed_sizes[] = {16, 24, 32, 40, 48, 64};
    int NUM_FIXED_SIZES = 6;
    void *addr;
    int alloced = 0;

    printf("Thread id: %d, %s\n", tid, __FUNCTION__);

    try {
        {
            Transaction tx1(_db, Transaction::ReadWrite);
            list<pair<void *, size_t>> tmp;

            // Test first fixed allocations
            printf("\nThread %d: Fixed size alloc - round 1\n", tid);
            for (int i = 0; i < NUM_FIXED_SIZES; ++i) {
                addr = _allocator->alloc(fixed_sizes[i]);
                tmp.push_back(pair<void *, size_t>(addr, fixed_sizes[i]));
            }
            tx1.commit();
            _freelocks[tid].lock();
            list<pair<void *, size_t>> &s = _tofree[tid];
            s.insert(s.end(), tmp.begin(), tmp.end());
            alloced += tmp.size();
            _freelocks[tid].unlock();
        }

        // Now test filling up a small chunk and then filling up the larger one
        // it came from
        // Use 64B chunk.
        unsigned index = 5;
        for (unsigned i = 0; i < 20; ++i) {
            Transaction tx(_db, Transaction::ReadWrite);
            list<pair<void *, size_t>> tmp;
            for (unsigned j = 0; j < 4016; ++j) {
                addr = _allocator->alloc(fixed_sizes[index]);
                tmp.push_back(pair<void *, size_t>(addr, fixed_sizes[index]));
            }
            tx.commit();
            _freelocks[tid].lock();
            list<pair<void *, size_t>> &s = _tofree[tid];
            s.insert(s.end(), tmp.begin(), tmp.end());
            alloced += tmp.size();
            _freelocks[tid].unlock();
            this_thread::sleep_for(chrono::microseconds(PEACE_TIME));
        }
        printf("Thread %d: Addr finally %p, total alloced %d\n", tid, addr, alloced);
    } catch (Exception e) {
        printf("Thread id: %d: ", tid);
        print_exception(e);
    }
    _allocdone[tid] = true;

    return 0;
}

int MTAllocTest::free_thread(int tid)
{
    int freed_t1 = 0, freed_t2 = 0;
    bool locked1 = false, locked2 = false;
    printf("Thread id: %d, %s\n", tid, __FUNCTION__);
    unsigned t1 = tid - 4, t2 = t1 + 2;

    // Allow for more time on slower systems.
    this_thread::sleep_for(chrono::microseconds(PEACE_TIME));
    printf("Free allocations from threads %d and %d\n", t1, t2);
    while (!_allocdone[t1] || !_allocdone[t2] || _tofree[t1].size() > 0 ||
            _tofree[t2].size() > 0) {
        try {
            _freelocks[t1].lock();  // unnecessary locking if lists empty but its ok.
            locked1 = true;
            if (_tofree[t1].size() > 0) {
                Transaction tx1(_db, Transaction::ReadWrite);
                for (auto it = _tofree[t1].begin(); it != _tofree[t1].end(); ++it) {
                    auto s = *it;
                    _allocator->free(s.first, s.second);
                }
                tx1.commit();
                freed_t1 += _tofree[t1].size();
                _tofree[t1].clear();
            }
            _freelocks[t1].unlock();
            locked1 = false;

            _freelocks[t2].lock();
            locked2 = true;
            if (_tofree[t2].size() > 0) {
                Transaction tx2(_db, Transaction::ReadWrite);
                for (auto it = _tofree[t2].begin(); it != _tofree[t2].end(); ++it) {
                    auto s = *it;
                    _allocator->free(s.first, s.second);
                }
                // No need to worry about unlocking due to transaction crash. The timeout
                // exception can happen only after commit.
                tx2.commit();
                freed_t2 += _tofree[t2].size();
                _tofree[t2].clear();
            }
            _freelocks[t2].unlock();
            locked2 = false;
            this_thread::sleep_for(chrono::microseconds(PEACE_TIME));
        } catch (Exception e) {
            printf("Thread id: %d: ", tid);
            print_exception(e);
            if (locked1)
                _freelocks[t1].unlock();
            if (locked2)
                _freelocks[t2].unlock();
            locked1 = locked2 = false;
        }
    }
    printf("Thread %d, freed for t1: %d, freed for t2: %d\n", tid, freed_t1, freed_t2);


    return 0;
}

int MTAllocTest::alloc_free_thread(int tid)
{
    long CHUNK_SIZE = 2 * 1024 * 1024;
    long size1M = 1024 * 1024;
    void *addr, *base;
    list<pair<void *, size_t>> tofree;

    printf("Thread id: %d, %s\n", tid, __FUNCTION__);

    for (unsigned i = 0; i < 5; ++i) {
        this_thread::sleep_for(chrono::microseconds(PEACE_TIME));
        try {
            {
                Transaction tx1(_db, Transaction::ReadWrite);
                addr = _allocator->alloc(3 * CHUNK_SIZE + size1M * 0.5);

                // Now see if a regular free form alloc comes from this page
                base = _allocator->alloc(size1M);
                tx1.commit();
                tofree.push_back(pair<void *, size_t>(addr, 3 * CHUNK_SIZE + size1M * 0.5));
            }
            {
                // Free the allocation before this and re-alloc from that space after commit
                Transaction tx(_db, Transaction::ReadWrite);
                _allocator->free(base, size1M);
                tx.commit();
            }
            {
                // Now allocate again, but just the smaller portion
                Transaction tx(_db, Transaction::ReadWrite);
                addr = _allocator->alloc(size1M * 0.5);
                tx.commit();
                tofree.push_back(pair<void *, size_t>(addr, size1M * 0.5));
            }

            this_thread::sleep_for(chrono::microseconds(PEACE_TIME));

            // Also, there will be no header for this one.
            {
                unsigned hdr_size = 24;
                // Testing borderline alloc within header size
                Transaction tx2(_db, Transaction::ReadWrite);
                addr = _allocator->alloc(CHUNK_SIZE - hdr_size);
                tx2.commit();
                tofree.push_back(pair<void *, size_t>(addr, CHUNK_SIZE - hdr_size));
            }
            {
                Transaction tx3(_db, Transaction::ReadWrite);
                addr = _allocator->alloc(3 * CHUNK_SIZE);
                tx3.commit();
            }
            {
                // Now test the free code.
                // Testing free for large borderline code
                Transaction tx4(_db, Transaction::ReadWrite);
                _allocator->free(addr, 3 * CHUNK_SIZE);

                // Next large alloc smaller than the block just freed will
                // still come from the tail and not the free list
                // Next alloc won't use free list but the previous pages have not been returned yet
                addr = _allocator->alloc(1.5 * CHUNK_SIZE);
                tx4.commit();
                tofree.push_back(pair<void *, size_t>(addr, 1.5 * CHUNK_SIZE));
            }
            {
                // But this one will
                Transaction tx5(_db, Transaction::ReadWrite);
                addr = _allocator->alloc(CHUNK_SIZE - 8);
                tx5.commit();
                tofree.push_back(pair<void *, size_t>(addr, CHUNK_SIZE - 8));
            }

            // Free all the addresses still allocated
            {
                Transaction tx6(_db, Transaction::ReadWrite);
                for (auto it = tofree.begin(); it != tofree.end(); ++it) {
                    auto s = *it;
                    _allocator->free(s.first, s.second);
                }
                tx6.commit();
                tofree.clear();
            }
            this_thread::sleep_for(chrono::microseconds(PEACE_TIME));
        } catch (Exception e) {
            printf("Thread id: %d: ", tid);
            print_exception(e);
        }
    }

    if (!tofree.empty()) {
        // Not really an error if timeouts prevent freeing unless we add
        // logic to keep retrying like in previous threads.
        printf("Thread %d: List still contains %ld elements\n", tid, tofree.size());
    }

    return 0;
}
