/**
 * @file   stripelocktest.cc
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
#include <string.h>
#include <thread>
#include <vector>
#include <unordered_set>
#include "stdlib.h"

#include "pmgd.h"
#include "../src/lock.h"
#include "util.h"

using namespace PMGD;
using namespace std;

static int simple_lock_test();
static int striped_lock_test();
static int mt_lock_test(unsigned num_threads);
static int run_test();

int main()
{
    return run_test();
}

int run_test()
{
    return simple_lock_test() + striped_lock_test() +  mt_lock_test(8);
}

int simple_lock_test()
{
    StripedLock lock(4096, 0);   // ==> 4096/2 = 2048 locks
    long data[4096] = { 0 };    // ==> 4096*8 = 32768B
    int retval = 0;

    // Maskbit = last 10 bits. So addresses at the same 10bits value
    // will clash.

    printf("Simple read/write lock test\n");

    // Simple read/write lock interaction with conflict
    unsigned long long id0, id1;
    id0 = lock.write_lock(&data[0]);
    try {
        id1 = lock.read_lock(&data[256]);
    } catch (Exception e) {
        printf("Expected exception\n");
        if (e.num != LockTimeout)
            retval += 1;
    }
    if (!lock.is_write_locked(id0)) {
        printf("Lock not write locked\n");
        retval += 1;
    }
    lock.write_unlock(id0);

    // Try upgrade lock
    try {
        id1 = lock.read_lock(&data[256]);
        if (lock.is_write_locked(id1)) {
            printf("How is write locked when only read asked for stripe %lld\n", id1);
            retval += 1;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        printf("Upgrade stripe id %lld to write lock\n", id1);
        lock.upgrade_lock(id1);
        if (!lock.is_write_locked(id1)) {
            printf("How is write NOT locked for stripe %lld\n", id1);
            retval += 1;
        }
        try {
            id0 = lock.read_lock(&data[0]);
        } catch (Exception e) {
            printf("Expected exception after upgrade\n");
            if (e.num != LockTimeout)
                retval += 1;
        }
        for (unsigned i = 0; i < 10000; ++i) {
            data[128] *= 2 * i;
        }
        lock.write_unlock(id1);

        try {
            id0 = lock.read_lock(&data[0]);
            lock.read_unlock(id0);
        } catch (Exception e) {
            printf("Unxpected exception after read lock of unlocked lock\n");
            retval += 1;
        }
    } catch (Exception e) {
        print_exception(e);
        retval += 1;
    }


    // Try consecutive read locks and then write.
    id0 = lock.read_lock(&data[1]);
    id1 = lock.read_lock(&data[257]);
    if (id0 != id1) {
        printf("Stripe ids don't overlap: %lld, %lld\n", id0, id1);
        retval++;
    }
    try {
        lock.upgrade_lock(id1);
    } catch (Exception e) {
        printf("Expected exception\n");
        if (e.num != LockTimeout)
            retval += 1;
    }
    lock.read_unlock(id0);
    try {
        lock.upgrade_lock(id1);
    } catch (Exception e) {
        printf("Unxpected exception after read lock of unlocked lock\n");
        retval += 1;
    }

    // Next line causes an assert.
    //lock.read_unlock(id1);
    lock.write_unlock(id1);
    if (lock.reader_count(id1) != 0 || lock.is_write_locked(id1)) {
        printf("Unlock mishandled\n");
        retval += 1;
    }

    return retval;
}

int striped_lock_test()
{
    StripedLock lock(4096, 64);   // ==> 4096/4 = 1024 locks
    char *data;
    int retval = 0;

    // Maskbit = last 10 bits. So addresses at the same 10bits value
    // will clash.

    // Test with a properly aligned address so the checks make sense.
    if (posix_memalign((void **)&data, 64, 4096 * sizeof(char)) < 0) {
        printf("Memory alloc error\n");
        retval++;
    }

    unsigned long long stripe_id = lock.get_stripe_id(&data[0]);
    printf("Stripe id for address %p = %lld\n", data, stripe_id);
    if (stripe_id != lock.get_stripe_id(&data[32])) {
        printf("Received a different stripe id when same expected\n");
        retval++;
    }
    if (stripe_id == lock.get_stripe_id(&data[64])) {
        printf("Received a same stripe id when different expected\n");
        retval++;
    }
    if (stripe_id != lock.get_stripe_id(&data[5])) {
        printf("Received a different stripe id when same expected for odd address\n");
        retval++;
    }

    return retval;
}


int run_writer_thread(long *data, StripedLock &lock, unsigned tid)
{
    unsigned low = tid * 1024, high = low + 1024;
    unordered_set<uint64_t> mylocks;

    printf("Writer thread: %d\n", tid);
    try {
        for (unsigned i = low; i < high; ++i) {
            if (mylocks.find(lock.get_stripe_id(&data[i])) == mylocks.end())
                mylocks.insert(lock.write_lock(&data[i]));
            data[i] = i * 2;
        }
    } catch (Exception e) {
        printf("Thread %d: ", tid);
        print_exception(e);
    }
    printf("Thread %d, acquired locks %ld\n", tid, mylocks.size());
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    for (auto it = mylocks.begin(); it != mylocks.end(); ++it)
        lock.write_unlock(*it);

    printf("Writer thread %d done\n", tid);
    return 0;
}

int run_reader_thread(long *data, StripedLock &lock, unsigned tid)
{
    unsigned low = tid * 1024, high = low + 1024;
    long myvar = 0;
    unordered_set<uint64_t> mylocks;

    printf("Reader thread: %d\n", tid);
    try {
        for (unsigned i = low; i < high; ++i) {
            if (mylocks.find(lock.get_stripe_id(&data[i])) == mylocks.end())
                mylocks.insert(lock.read_lock(&data[i]));
            myvar += data[i];
        }
    } catch (Exception e) {
        printf("RThread %d: ", tid);
        print_exception(e);
    }
    printf("RThread %d, acquired locks %ld\n", tid, mylocks.size());
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    for (auto it = mylocks.begin(); it != mylocks.end(); ++it)
        lock.read_unlock(*it);

    printf("Reader thread %d done, myvar: %ld\n", tid, myvar);
    return 0;
}

int mt_lock_test(unsigned num_threads)
{
    vector<thread> threads;
    StripedLock lock(4096, 0);
    long *data = (long *)calloc(num_threads * 1024 / 2, sizeof(long));

    for (unsigned i = 0; i < num_threads / 2; ++i)
        threads.push_back(thread(&run_writer_thread, data, ref(lock), i));

    for (unsigned i = 0; i < num_threads / 2; ++i)
        threads.push_back(thread(&run_reader_thread, data, ref(lock), i));

    for (auto &th : threads)
        th.join();
    return 0;
}
