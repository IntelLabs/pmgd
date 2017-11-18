/**
 * @file   allocaborttest.cc
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

/*
 * This unit test exercises the fixed-size allocator.
 */

#include <iostream>
#include <string.h>
#include "pmgd.h"
#include "util.h"
#include "../src/Allocator.h"
#include "../src/os.h"
#include "exception.h"

using namespace PMGD;

namespace PMGD {
    class AllocAbortTest
    {
        int r;

        void var_freeform_tests(Graph &db, Allocator &allocator1);
        void var_fixed_tests(Graph &db, Allocator &allocator1);
        void var_large_tests(Graph &db, Allocator &allocator1);
        void passfail(long id, long expected, long actual);
    public:
        AllocAbortTest() : r(0) {}
        int fixed_allocator_test();
        int var_allocator_test();
    };
}

int main(int argc, char **argv)
{
    std::cout << "Allocator unit test\n\n";
    AllocAbortTest at;
    return at.fixed_allocator_test() + at.var_allocator_test();
}

int AllocAbortTest::fixed_allocator_test()
{
    uint64_t start_addr;
    uint64_t pool_size;
    unsigned object_size;

    try {
        Graph db("fixedallocabortgraph", Graph::Create);
        Transaction txa(db, Transaction::ReadWrite);

        bool create1 = true;
        start_addr = 0x200000000;
        pool_size = 1024;
        object_size = 32;
        os::MapRegion region1("fixedallocabortgraph", "region1", start_addr, pool_size, create1, create1, false);
        CommonParams params(create1, false);
        FixedAllocator allocator1(start_addr, object_size, pool_size, params);
        txa.commit();

        long base1 = start_addr + /* sizeof(struct RegionHeader) */64;
        int index = 0, testcount = 0;
        void *addr[6];

        try {
            Transaction tx(db, Transaction::ReadWrite);
            addr[index] = allocator1.alloc();
            passfail(testcount++, base1, (long)addr[index]);
            index++;
            addr[index] = allocator1.alloc();
            passfail(testcount++, base1 + index * object_size, (long)addr[index]);
            index++;
            addr[index] = allocator1.alloc();
            passfail(testcount++, base1 + index * object_size, (long)addr[index]);
            index = 0;
            throw PMGDException(UndefinedException);
        }
        catch(Exception e) {
        }
        {
            Transaction tx(db, Transaction::ReadWrite);
            addr[index] = allocator1.alloc();
            passfail(testcount++, base1, (long)addr[index]);
            index++;
            addr[index] = allocator1.alloc();
            passfail(testcount++, base1 + index * object_size, (long)addr[index]);
            index++;
            addr[index] = allocator1.alloc();
            passfail(testcount++, base1 + index * object_size, (long)addr[index]);
            index++;
            tx.commit();
        }
        {
            Transaction tx(db, Transaction::ReadWrite);
            allocator1.free(addr[1]);
            allocator1.free(addr[0]);
        }
        {
            Transaction tx(db, Transaction::ReadWrite);
            addr[index] = allocator1.alloc();
            passfail(testcount++, base1 + index * object_size, (long)addr[index]);
            index++;
            addr[index] = allocator1.alloc();
            passfail(testcount++, base1 + index * object_size, (long)addr[index]);
            index++;
            addr[index] = allocator1.alloc();
            passfail(testcount++, base1 + index * object_size, (long)addr[index]);
            index++;
            tx.commit();
        }
        {
            Transaction tx(db, Transaction::ReadWrite);
            allocator1.free(addr[1]);
            allocator1.free(addr[0]);
            tx.commit();
        }
        {
            Transaction tx(db, Transaction::ReadWrite);
            addr[0] = allocator1.alloc();
            passfail(testcount++, base1 + object_size, (long)addr[0]);
            addr[1] = allocator1.alloc();
            passfail(testcount++, base1, (long)addr[1]);
            tx.commit();
        }
    }
    catch (Exception e)
    {
        print_exception(e);
        return 1;
    }

    printf("Fixed allocator abort tests done...\n\n");
    return r;
}

uint64_t start_addr;
uint64_t pool_size;
unsigned hdr_size;
int testnum = 1;

void AllocAbortTest::var_freeform_tests(Graph &db, Allocator &allocator1)
{
    long CHUNK_SIZE = 2 * 1024 * 1024;
    long size1M = 1024 * 1024;
    hdr_size = 24;
    void *addr = NULL;
    long base = start_addr + CHUNK_SIZE;  // 1 fixed chunk allocated at init time

    printf("FOR ABORT: Test basic allocations that fill up one chunk\n");
    {
        Transaction tx1(db, Transaction::ReadWrite);
        // allocating a MB to start filling up the chunk.
        addr = allocator1.alloc(size1M);
        passfail(testnum++, base + CHUNK_SIZE - size1M, (long)addr);
        // Now fill up the chunk
        addr = allocator1.alloc(size1M - hdr_size);
        passfail(testnum++, base + hdr_size, (long)addr);
        printf("New page at 0x%lx\n", base);

        base += CHUNK_SIZE;
        // The next set of allocations should nicely go to another chunk
        printf("Now test allocations to come from the next (new) chunk\n");
        addr = allocator1.alloc(size1M);
        printf("New page at 0x%lx\n", base);
        passfail(testnum++, base + CHUNK_SIZE - size1M, (long)addr);

        printf("Allocate more than the current chunk can support\n");
        base += CHUNK_SIZE;
        // Allocate another 1.5MB + 4K (so the test afterwards has to go to
        // the second chunk and not third) and it should go in a new chunk.
        addr = allocator1.alloc(size1M * 1.5 + 4096);
        printf("New page at 0x%lx\n", base);
        passfail(testnum++, base + CHUNK_SIZE - (size1M * 1.5 + 4096), (long)addr);
    }

    // Should reset base to what it used to be.
    base = start_addr + CHUNK_SIZE;
    printf("For COMMIT: Test basic allocations that fill up one chunk\n");
    Transaction tx2(db, Transaction::ReadWrite);
    // allocating a MB to start filling up the chunk.
    addr = allocator1.alloc(size1M);
    passfail(testnum++, base + CHUNK_SIZE - size1M, (long)addr);
    // Now fill up the chunk
    addr = allocator1.alloc(size1M - hdr_size);
    passfail(testnum++, base + hdr_size, (long)addr);
    printf("New page at 0x%lx\n", base);

    base += CHUNK_SIZE;
    // The next set of allocations should nicely go to another chunk
    printf("Now test allocations to come from the next (new) chunk\n");
    addr = allocator1.alloc(size1M);
    printf("New page at 0x%lx\n", base);
    passfail(testnum++, base + CHUNK_SIZE - size1M, (long)addr);

    printf("Allocate more than the current chunk can support\n");
    base += CHUNK_SIZE;
    // Allocate another 1.5MB + 4K (so the test afterwards has to go to
    // the second chunk and not third) and it should go in a new chunk.
    addr = allocator1.alloc(size1M * 1.5 + 4096);
    printf("New page at 0x%lx\n", base);
    passfail(testnum++, base + CHUNK_SIZE - (size1M * 1.5 + 4096), (long)addr);
    tx2.commit();

    printf("Variable allocator freeform abort tests done....\n");
    // At the end of this test, there should be four 2MB chunks occupied.
    // One by the default allocations for fixed chunks and 3 for this test.
}

void AllocAbortTest::var_fixed_tests(Graph &db, Allocator &allocator1)
{
    unsigned fixed_sizes[] = {16, 24, 32, 40, 48, 64};
    int NUM_FIXED_SIZES = 6;
    // sizeof(struct FixedChunk(16)) + align
    long hdr_offset[] = {64, 64, 64, 72, 80, 64};
    //long max_objects[] = {48, 40, 32, 64, 64, 64};

    // Number of objects allocated
    long SMALL_CHUNK_SIZE = 4096;
    //long CHUNK_SIZE = 2 * 1024 * 1024;

    // Allocate one object in each size and see how the rollback goes.
    {
        Transaction tx(db, Transaction::ReadWrite);
        // Test first fixed allocations
        printf("\nFixed size alloc - round 1 abort\n");
        for (int i = 0; i < NUM_FIXED_SIZES; ++i) {
            long base = start_addr + hdr_offset[i];
            base += SMALL_CHUNK_SIZE*i;
            void *addr = allocator1.alloc(fixed_sizes[i]);
            passfail(testnum++, base, (long)addr);
            addr = allocator1.alloc(fixed_sizes[i]);
            addr = allocator1.alloc(fixed_sizes[i]);
        }
    }
    {
        Transaction tx(db, Transaction::ReadWrite);
        printf("Fixed size alloc - round 1 commit - reverse order of sizes\n");
        for (int i = 0; i < NUM_FIXED_SIZES; ++i) {
            long base = start_addr + hdr_offset[i];
            base += SMALL_CHUNK_SIZE * i;
            void *addr = allocator1.alloc(fixed_sizes[i]);
            passfail(testnum++, base, (long)addr);
        }
        tx.commit();
    }
    // Allocate some size and almost fill up the 2MB page to test crossing
    // over when we allocate 64B pages. Also, round allocations to a page
    // boundary
    // Use 64B chunk.
    unsigned index = 5;
    void *addr;
    {
        printf("Fill large page with 64B chunks\n");
        for (int j = 0; j < 5; ++j) {
            Transaction tx(db, Transaction::ReadWrite);
            for (int i = 0; i < 3238; ++i)
                addr = allocator1.alloc(fixed_sizes[index]);
            tx.commit();
        }
    }
    printf("Allocated till: %p\n", addr);

    int allocated = 0, num_pages = 1;
    long base = (long)addr + hdr_offset[index] + fixed_sizes[index];
    printf("Base before aborted allocation: 0x%lx\n", base);
    try {
        printf("Fixed size alloc - fill large page with 64B chunks - abort\n");
        Transaction tx(db, Transaction::ReadWrite);
        for (int i = 0; i < 16255; ++i) {
            addr = allocator1.alloc(fixed_sizes[index]);
            allocated++;
            if (i == 0)
                passfail(testnum++, base, (long)addr);
            if ( ((long)addr & (SMALL_CHUNK_SIZE - 1)) == hdr_offset[index]) {
                base = (long)addr - hdr_offset[index];
                num_pages++;
            }
        }
    }
    catch (Exception e) {
        print_exception(e);
    }
    printf("Base at exception: 0x%lx\n", base);
    // With an abort, pages are inserted in the DRAM set in some random set
    // order. After examining the top of the set with this particular example,
    // we know it is 8001ab040
    long base64 = start_addr + 0x1ab040;
    {
        printf("Fixed size alloc - fill large page with 64B chunks\n");
        Transaction tx(db, Transaction::ReadWrite);
        //base64 += hdr_offset[index];
        for (int i = 0; i < 250; ++i) {
            addr = allocator1.alloc(fixed_sizes[index]);
            if (i == 0)
                passfail(testnum++, base64, (long)addr);
        }
        tx.commit();
        printf("Base: 0x%lx, addr: %p\n", base64, addr);
    }


    printf("Variable allocator fixed tests done....\n");
}


void AllocAbortTest::var_large_tests(Graph &db, Allocator &allocator1)
{
    long CHUNK_SIZE = 2 * 1024 * 1024;
    long size1M = 1024 * 1024;
    // There is a static allocation of one page at allocator creation.
    long start_base = start_addr + CHUNK_SIZE;
    void *addr;

    long base = start_base + CHUNK_SIZE - size1M * 0.5;
    printf("\nTesting large alloc, ABORT first\n");
    {
        Transaction tx(db, Transaction::ReadWrite);
        addr = allocator1.alloc(3 * CHUNK_SIZE + size1M * 0.5);
        passfail(testnum++, base, (long)addr);

        // Now see if a regular free form alloc comes from this page
        printf("Testing a regular alloc from the large block\n");
        base = start_base + CHUNK_SIZE - size1M * 1.5;
        addr = allocator1.alloc(size1M);
        passfail(testnum++, base, (long)addr);
    }
    {
        base = start_base + CHUNK_SIZE - size1M * 0.5;
        Transaction tx(db, Transaction::ReadWrite);
        addr = allocator1.alloc(3 * CHUNK_SIZE + size1M * 0.5);
        passfail(testnum++, base, (long)addr);

        // Now see if a regular free form alloc comes from this page
        printf("Testing a regular alloc from the large block\n");
        base = start_base + CHUNK_SIZE - size1M * 1.5;
        addr = allocator1.alloc(size1M);
        passfail(testnum++, base, (long)addr);
        tx.commit();
    }

    printf("base after large allocs: 0x%lx\n", base);

    // Also, there will be no header for this one.
    base = start_base + 4 * CHUNK_SIZE;
    hdr_size = 24;
    printf("Testing borderline alloc within header size, ABORT first\n");
    {
        Transaction tx2(db, Transaction::ReadWrite);
        addr = allocator1.alloc(CHUNK_SIZE - hdr_size);
        passfail(testnum++, base, (long)addr);
    }
    {
        Transaction tx2(db, Transaction::ReadWrite);
        addr = allocator1.alloc(CHUNK_SIZE - hdr_size);
        passfail(testnum++, base, (long)addr);
        tx2.commit();
    }

    printf("base after borderline allocs: 0x%lx\n", base);

    // Now test the free code.
    printf("Testing free for large borderline code after large alloc\n");
    base += CHUNK_SIZE;
    {
        Transaction tx3(db, Transaction::ReadWrite);
        addr = allocator1.alloc(3 * CHUNK_SIZE);
        passfail(testnum++, base, (long)addr);
        tx3.commit();
    }
    {
        printf("ABORT free case first\n");
        Transaction tx4(db, Transaction::ReadWrite);
        allocator1.free((void *)base, 3 * CHUNK_SIZE);

        // Next large alloc smaller than the block just freed will
        // still come from the tail and not the free list
        base += 4*CHUNK_SIZE;
        printf("Next alloc won't use free list but the previous pages have not been returned yet\n");
        addr = allocator1.alloc(1.5 * CHUNK_SIZE);
        passfail(testnum++, base - size1M, (long)addr);
    }
    {
        base -= 4*CHUNK_SIZE;
        printf("COMMIT free case next\n");
        Transaction tx4(db, Transaction::ReadWrite);
        allocator1.free((void *)base, 3 * CHUNK_SIZE);

        // Next large alloc smaller than the block just freed will
        // still come from the tail and not the free list
        base += 4*CHUNK_SIZE;
        printf("Next alloc won't use free list but the previous pages have not been returned yet\n");
        addr = allocator1.alloc(1.5 * CHUNK_SIZE);
        passfail(testnum++, base - size1M, (long)addr);
        tx4.commit();
    }

    printf("Large tests done....\n");
}

int AllocAbortTest::var_allocator_test()
{
    try {
        Graph::Config config;
        config.allocator_region_size = 104857600;  // 100MB
        {
            Graph db("varallocabortgraph", Graph::Create, &config);
            Allocator *allocator1 = Allocator::get_main_allocator(db);
            start_addr = allocator1->get_start_addr();

            // While testing, also hit the case when the pool limit reaches.
            // That works but did not keep the test case here.
            var_freeform_tests(db, *allocator1);

            var_fixed_tests(db, *allocator1);
        }

        {
            Graph dbl("varallocabortlargegraph", Graph::Create, &config);
            Allocator *allocator2 = Allocator::get_main_allocator(dbl);
            start_addr = allocator2->get_start_addr();

            // Now test the large and borderline allocations
            var_large_tests(dbl, *allocator2);
        }
    }
    catch (Exception e)
    {
        print_exception(e);
        return 1;
    }

    return r;
}

void AllocAbortTest::passfail(long id, long expected, long actual)
{
    if (expected == actual) {
        std::cout << "Test " << id << ": passed\n";
        return;
    }

    std::cerr << "Test " << id << ": failed: ";
    std::cerr << "expected " << std::hex << expected << "; ";
    std::cerr << "got " << std::hex << actual << "\n";

    r++;
}
