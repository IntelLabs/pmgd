/**
 * @file   alloctest.cc
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
#include "../src/GraphConfig.h"
#include "../src/os.h"
#include "exception.h"

using namespace PMGD;

namespace PMGD {
    class AllocTest
    {
        int r;

        void var_freeform_tests(Graph &db, Allocator &allocator1);
        void var_fixed_tests(Graph &db, Allocator &allocator1);
        void var_large_tests(Graph &db, Allocator &allocator1);
        void passfail(long id, long expected, long actual);
    public:
        AllocTest() : r(0) {}
        int fixed_allocator_test();
        int var_allocator_test();
    };
}

static std::ostream& operator<< (std::ostream &out, Exception& e);

int main(int argc, char **argv)
{
    std::cout << "Allocator unit test\n\n";
    AllocTest at;
    return at.fixed_allocator_test() + at.var_allocator_test();
}

int AllocTest::fixed_allocator_test()
{
    uint64_t start_addr;
    uint64_t pool_size;
    unsigned object_size;

    try {
        Graph db("fixedallocgraph", Graph::Create);
        Transaction tx1(db, Transaction::ReadWrite);

        bool create1 = true;
        start_addr = 0x200000000;
        pool_size = 1024;
        object_size = 32;
        os::MapRegion region1("fixedallocgraph", "region1", start_addr, pool_size, create1, create1, false);
        CommonParams params(create1, false);
        FixedAllocator allocator1(start_addr, object_size, pool_size, params);
        long base1 = start_addr + /* sizeof(struct RegionHeader) */64;

        void *addr1 = allocator1.alloc();
        passfail(1, base1, (long)addr1);

        void *addr2 = allocator1.alloc();
        passfail(2, base1 + object_size, (long)addr2);

        tx1.commit();

        Transaction tx2(db, Transaction::ReadWrite);

        allocator1.free(addr2);
        allocator1.free(addr1);

        tx2.commit();

        Transaction tx3(db, Transaction::ReadWrite);

        addr2 = allocator1.alloc();
        passfail(3, base1 + object_size, (long)addr2);

        addr1 = allocator1.alloc();
        passfail(4, base1, (long)addr1);

        void *addr3 = allocator1.alloc();
        passfail(5, base1 + 2*object_size, (long)addr3);


        bool create2 = true;
        start_addr = 0x300000000;
        pool_size = 96;
        object_size = 32;
        os::MapRegion region2("fixedallocgraph", "region2", start_addr, pool_size, create2, create2, false);
        FixedAllocator allocator2(start_addr, object_size, pool_size, params);
        long base2 = start_addr + /* sizeof(struct RegionHeader) */64;

        addr1 = allocator2.alloc();
        passfail(6, base2, (long)addr1);

        bool ok = false;
        try {
            addr2 = allocator2.alloc();
        }
        catch (Exception e)
        {
            ok = true;
            std::cout << e << "\n";
            std::cout << "Test 7: passed\n";
        }
        if (!ok) {
            std::cerr << "Test 7: failed: failure to generate an exception\n";
            exit(1);
        }

        tx3.commit();
    }
    catch (Exception e)
    {
        print_exception(e);
        return 1;
    }

    printf("Fixed allocator tests done...\n\n");
    return r;
}

uint64_t start_addr;
uint64_t pool_size;
unsigned hdr_size;
int testnum = 1;

void AllocTest::var_freeform_tests(Graph &db, Allocator &allocator1)
{
    long CHUNK_SIZE = 2 * 1024 * 1024;
    long size1M = 1024 * 1024;
    hdr_size = 24;
    void *addr = NULL;
    long base = start_addr + CHUNK_SIZE;  // 1 fixed chunk allocated at init time
    long chunk_start = base;

    printf("Test basic allocations that fill up one chunk\n");
    Transaction tx1(db, Transaction::ReadWrite);
    // allocating a MB to start filling up the chunk.
    addr = allocator1.alloc(size1M);
    passfail(testnum++, base + CHUNK_SIZE - size1M, (long)addr);
    // Now fill up the chunk
    addr = allocator1.alloc(size1M - hdr_size);
    passfail(testnum++, base + hdr_size, (long)addr);
    printf("New page at 0x%lx\n", base);
    tx1.commit();

    base += CHUNK_SIZE;
    // The next set of allocations should nicely go to another chunk
    printf("Now test allocations to come from the next (new) chunk\n");
    Transaction tx2(db, Transaction::ReadWrite);
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

    // Next allocation should come from second chunk in this test
    // since that's the only one with enough space for this
    printf("Allocate such that the second chunk can fit it but not third\n");
    addr = allocator1.alloc(size1M * 0.5);
    base = chunk_start + CHUNK_SIZE;
    // already one 1M allocation there but we do backward allocs
    passfail(testnum++, base + size1M*0.5 , (long)addr);

    // So now, the second chunk has 1572896B allocated, 524256 free
    // This next allocation should remove it from the available list
    printf("The next allocation should still fit in second chunk, not in third\n");
    base = base + CHUNK_SIZE - size1M * 1.5;
    addr = allocator1.alloc(524250);
    passfail(testnum++, base - 524250, (long)addr);

    // The next allocation will be able to fit in the third chunk (520160B left).
    // First and second should now be filled up.
    printf("Last allocation from third chunk\n");
    base = chunk_start + 2*CHUNK_SIZE + CHUNK_SIZE - (size1M * 1.5 + 4096);
    addr = allocator1.alloc(520160);
    passfail(testnum++, base - 520160, (long)addr);
    tx2.commit();

    // Now free the previous block of 1MB to test if a new 1MB
    // allocation can go back there. It won't fit in the last chunk
    // allocated since there is 32B less available there.
    Transaction tx3(db, Transaction::ReadWrite);
    {
        Transaction tx(db, Transaction::ReadWrite | Transaction::Independent);
        printf("Now free the >1.5M allocation from chunk 3 in an independent transaction\n");
        base = chunk_start + 2*CHUNK_SIZE + CHUNK_SIZE - (size1M * 1.5 + 4096);
        allocator1.free((void *)base, (size1M *1.5 + 4096));
        tx.commit();
    }

    printf("Test if the free worked by allocating a new chunk that should fit\n");
    base += 4096 + size1M * 0.5;
    addr = allocator1.alloc(size1M);
    passfail(testnum++, base, (long)addr);

    printf("Allocate another 0.5M followed by 4K to refill the space\n");
    base -= size1M*0.5;
    addr = allocator1.alloc(size1M * 0.5);
    passfail(testnum++, base, (long)addr);

#ifdef INNER_TX_CORRECT
    /**** Is this test case, and therefore the underlying logic, important?
     * Right now, modifying it to let the original sequence of tests be as is.
     */
    printf("Test the simple coalescing by freeing the last allocation from chunk3 + 4k\n");
    {
        Transaction tx(db, Transaction::ReadWrite | Transaction::Independent);
        base -= 4096;
        base -= 520160;
        allocator1.free((void *)base, 520160);
        tx.commit();
    }
    addr = allocator1.alloc(4096 + 520160);
    passfail(testnum++, base, (long)addr);
    tx3.commit();
#else
    tx3.commit();

    printf("Test the simple coalescing by freeing the last allocation from chunk3 + 4k\n");
    {
        Transaction tx(db, Transaction::ReadWrite | Transaction::Independent);
        base -= 4096;
        base -= 520160;
        allocator1.free((void *)base, 520160);
        tx.commit();
    }
    {
        Transaction tx(db, Transaction::ReadWrite);
        addr = allocator1.alloc(4096 + 520160);
        passfail(testnum++, base, (long)addr);
        tx.commit();
    }
#endif

    // Test releasing all allocations from the first chunk to see if it
    // is returned to the free pool and then reused.
    Transaction tx4(db, Transaction::ReadWrite);
    printf("Testing return of 2MB chunks to FixedAllocator\n");
    base = chunk_start;
    allocator1.free((void *)(base + hdr_size), size1M - hdr_size);
    base += size1M;
    allocator1.free((void *)base, size1M);
    tx4.commit();

    // Have to change transaction to test this since the FixedAllocator
    // won't reuse the free page within the same transaction
    Transaction tx5(db, Transaction::ReadWrite);
    base = chunk_start + hdr_size + 8;
    // Subtract an extra 8 to avoid borderline allocation
    addr = allocator1.alloc(CHUNK_SIZE - hdr_size - 8);
    passfail(testnum++, base, (long)addr);
    tx5.commit();

    printf("Variable allocator freeform tests done....\n");
    // At the end of this test, there should be four 2MB chunks occupied.
    // One by the default allocations for fixed chunks and 3 for this test.
}

void AllocTest::var_fixed_tests(Graph &db, Allocator &allocator1)
{
    unsigned fixed_sizes[] = {16, 24, 32, 40, 48, 64};
    int NUM_FIXED_SIZES = 6;
    // sizeof(struct FixedChunk(16)) + align
    long hdr_offset[] = {64, 64, 64, 72, 80, 64};
    //long max_objects[] = {48, 40, 32, 64, 64, 64};

    // Number of objects allocated
    long num_objects = 0;
    long SMALL_CHUNK_SIZE = 4096;
    long CHUNK_SIZE = 2 * 1024 * 1024;

    Transaction tx1(db, Transaction::ReadWrite);
    // Test first fixed allocations
    printf("\nFixed size alloc - round 1\n");
    for (int i = 0; i < NUM_FIXED_SIZES; ++i) {
        long base = start_addr + hdr_offset[i] + num_objects*fixed_sizes[i];
        base += SMALL_CHUNK_SIZE*i;
        void *addr = allocator1.alloc(fixed_sizes[i]);
        passfail(testnum++, base, (long)addr);
    }
    num_objects++;
    tx1.commit();

    Transaction tx2(db, Transaction::ReadWrite);
    //  Test second allocations, across transaction boundaries.
    printf("Fixed size alloc - round 2. Another transaction\n");
    for (int i = 0; i < NUM_FIXED_SIZES; ++i) {
        long base = start_addr + hdr_offset[i] + num_objects*fixed_sizes[i];
        base += SMALL_CHUNK_SIZE*i;
        void *addr = allocator1.alloc(fixed_sizes[i]);
        passfail(testnum++, base, (long)addr);
    }
    num_objects++;
    tx2.commit();

    Transaction tx3(db, Transaction::ReadWrite);
    //  Test allocations after free
    printf("Fixed size free then alloc\n");
    for (int i = 0; i < NUM_FIXED_SIZES; ++i) {
        long base = start_addr + hdr_offset[i] + (num_objects - 1)*fixed_sizes[i];
        base += SMALL_CHUNK_SIZE*i;
        allocator1.free((void *)base, fixed_sizes[i]);

        // The address here should come from the next index cause we keep moving
        // the index until chunk is full.
        base = start_addr + hdr_offset[i] + num_objects*fixed_sizes[i];
        base += SMALL_CHUNK_SIZE*i;
        void *addr = allocator1.alloc(fixed_sizes[i]);
        passfail(testnum++, base, (long)addr);
    }
    num_objects++;
    tx3.commit();

    // Now test filling up a small chunk and then filling up the larger one
    // it came from.
    Transaction tx4(db, Transaction::ReadWrite);
    //  Test second allocations, across transaction boundaries.
    printf("Fixed size alloc - fill 64B chunks\n");
    // Use 64B chunk.
    // The active one already has 2 allocations and one spot is used by
    // the header. This should also test the move of next_index when its
    // next bitmap entry is full.
    unsigned index = 5;
    void *addr;
    // The base for the last object allocated here should be that free
    // spot left by second allocation above.
    long base = start_addr + hdr_offset[index] + (num_objects - 2)*fixed_sizes[index];
    base += SMALL_CHUNK_SIZE*index;
    void *addr0 = NULL;
    long base0 = base + 2 * fixed_sizes[index];  // First address allocated here from 4th spot
    for (int i = 0; i < 61; ++i) {
        addr = allocator1.alloc(fixed_sizes[index]);
        if (i == 0)
            addr0 = addr;
    }
    printf("Base0: 0x%lx, addr0: %p\n", base0, addr0);
    printf("Base: 0x%lx, addr: %p\n", base, addr);
    passfail(testnum++, base0, (long)addr0);
    passfail(testnum++, base, (long)addr);
    tx4.commit();

    // Now free all the 64B allocations.
    printf("Testing all frees for 64B and then one alloc\n");
    base = start_addr + hdr_offset[index];
    base += SMALL_CHUNK_SIZE*index;
    Transaction tx5(db, Transaction::ReadWrite);
    for (int i = 0; i < 63; ++i) {
        allocator1.free((void *)base, fixed_sizes[index]);
        base += fixed_sizes[index];
    }
    // Commit here so that the frees actually take effect. Even the
    // starting chunk is freed now.
    tx5.commit();

    Transaction tx6(db, Transaction::ReadWrite);
    // Now allocate one more and make sure we get the 0th index
    base -= 63*fixed_sizes[index];
    printf("Base at this point: 0x%lx\n", base);
    addr = allocator1.alloc(fixed_sizes[index]);
    passfail(testnum++, base, (long)addr);
    tx6.commit();

    long endbase64 = start_addr;
    printf("Now allocate 64B all the way to filling the entire 2MB chunk\n");
    // We have 1 allocation in the 64B chunk. Each 2MB page can contain 512
    // 4K chunks of which we are currently using 6 (including 1 for 64B), so 506.
    // So 507 chunks can contain 31941 allocations of 64B. To cross over to the next
    // 2MB page, allocate at least 31942. Actually filling 2 2MB pages so
    // we can do a free test. So add another 32256
    Transaction tx7(db, Transaction::ReadWrite);
    for (int i = 1; i < 64256; ++i) {
        // Accounts for the one allocated already
        base += fixed_sizes[index];
        // Running out of journal space
        Transaction tx(db, Transaction::ReadWrite | Transaction::Independent);
        addr = allocator1.alloc(fixed_sizes[index]);
        // Note down how many page switches
        if ( (long)addr % CHUNK_SIZE == hdr_offset[index] )
            printf("Switched to next 2MB at %d, addr: %p\n", i, addr);
        // Since the variable alloc test happens before this, the address of the
        // next one will come 4 chunks after
        if (addr == (void *)(start_addr + 6*CHUNK_SIZE + hdr_offset[index])) {
            base = (long)addr;
            endbase64 = base - hdr_offset[index];
            printf("Switched to next 2MB at %d, endbase64: 0x%lx\n", i, endbase64);
        }
        tx.commit();
    }
    printf("Base at this point: 0x%lx\n", base);
    passfail(testnum++, base, (long)addr);
    tx7.commit();

    // Test maxing out another size, not a power of 2.
    printf("Now allocate 48B all the way to filling the entire 2MB chunk\n");
    // We have 2 allocations in the 48B chunk. Each 2MB page can contain 512
    // 4K chunks of which we are currently using 1 (from the previous test), so 511.
    // So 511 chunks can contain 42413 allocations of 48B. To cross over to the next
    // 2MB page, allocate at least 42450. Also, there are still 81 spots left in the
    // first chunk dedicated to 48B. Rounding up to 42500.
    Transaction tx8(db, Transaction::ReadWrite);
    unsigned index48 = 4;
    for (int i = 2; i < 42500; ++i) {
        // Accounts for the one allocated already
        base += fixed_sizes[index48];
        // Running out of journal space
        Transaction tx(db, Transaction::ReadWrite | Transaction::Independent);
        addr = allocator1.alloc(fixed_sizes[index48]);
        if ( (long)addr % 4096 == hdr_offset[index] )
            base += hdr_offset[index48];
        // Added two pages to the earlier base
        if (addr == (void *)(endbase64 + CHUNK_SIZE + hdr_offset[index48])) {
            base = (long)addr;
            printf("Switched to next 2MB at %d, base now: 0x%lx\n", i, base);
        }
        tx.commit();
    }
    printf("Base at this point: 0x%lx\n", base);
    passfail(testnum++, base, (long)addr);

    printf("Test return of all 48B emptying a 2MB page but shouldn't be returned cause it is current\n");
    // Now free enough of the 48B allocations to check if the 2MB page
    // is returned to the allocator. The above loop switches to a new
    // 2MB page at 42496. So freeing 4 allocations will empty that page.
    {
        Transaction tx(db, Transaction::ReadWrite | Transaction::Independent);
        for (int i = 0; i < 4; ++i) {
            allocator1.free((void *)base, fixed_sizes[index48]);
            base -= fixed_sizes[index48];
        }
        tx.commit();
    }
    printf("Base at this point: 0x%lx\n", base);
    printf("Now free/alloc straddling to previous page\n");
    // Base is moved a little extra behind in the previous for. So compensate.
    long new_page = base + fixed_sizes[index48] - hdr_offset[index48];
    printf("new_page set to 0x%lx, and base at 0x%lx\n", new_page, base);
    // 64B already occupies a00000 and c00000
    // So free some old address and see if that is not what is allocated first.
    base = start_addr + 0x4050;
    {
        Transaction tx(db, Transaction::ReadWrite | Transaction::Independent);
        allocator1.free((void *)base, fixed_sizes[index48]);
        tx.commit();
    }
    addr = allocator1.alloc(fixed_sizes[index48]);
    // The new alloc will come from the latest page where we freed since
    // it had 0 spaces before and by creating a space, we moved it to the
    // list of available pages.
    passfail(testnum++, base, (long)addr);
    printf("New page at 48B allocations: 0x%lx, and base: 0x%lx\n", new_page, base);
    tx8.commit();

    // The base at 0x800a00000 has only 64B fixed allocs and is
    // not the current one for 64B. So test the return of 2MB
    // chunk using that. There are 32256 allocations to free.
    // Each page has 63 allocations.
    printf("Test returning of main chunk to the main allocator\n");
    Transaction tx9(db, Transaction::ReadWrite);
    base = start_addr + 0xa00000;
    for (int i = 0; i < 512; ++i) {
        // Running out of journal space
        Transaction tx(db, Transaction::ReadWrite | Transaction::Independent);
        base += hdr_offset[index];
        for (int j = 0; j < 63; ++j) {
            allocator1.free((void *)base, fixed_sizes[index]);
            base += fixed_sizes[index];
        }
        tx.commit();
    }
    tx9.commit();

    printf("Variable allocator fixed tests done....\n");
}


void AllocTest::var_large_tests(Graph &db, Allocator &allocator1)
{
    long CHUNK_SIZE = 2 * 1024 * 1024;
    long size1M = 1024 * 1024;
    // The previous tests run up to 0xc00000
    long start_base = start_addr + 0xe00000;
    void *addr;

    // Base will be the contiguous one, not the ones freed in between,
    // unless the requested size fits within 2MB chunk like for our first
    // example.
    long base = start_base + CHUNK_SIZE - size1M * 0.5;
    printf("\nTesting large alloc\n");
    Transaction tx1(db, Transaction::ReadWrite);
    addr = allocator1.alloc(3 * CHUNK_SIZE + size1M * 0.5);
    passfail(testnum++, base, (long)addr);

    // Now see if a regular free form alloc comes from this page
    printf("Testing a regular alloc from the large block\n");
    base = start_base + CHUNK_SIZE - size1M * 1.5;
    addr = allocator1.alloc(size1M);
    passfail(testnum++, base, (long)addr);

#ifdef INNER_TX_CORRECT
    {
        printf("Free the allocation before this and re-alloc from that space after commit\n");
        Transaction tx(db, Transaction::ReadWrite | Transaction::Independent);
        base = start_base + CHUNK_SIZE - size1M * 1.5;
        allocator1.free((void *)base, size1M);
        tx.commit();
    }
    printf("Now allocate again, but just the smaller portion\n");
    addr = allocator1.alloc(size1M * 0.5);
    passfail(testnum++, base, (long)addr);
    tx1.commit();
#else
    tx1.commit();
    {
        printf("Free the allocation before this and re-alloc from that space after commit\n");
        Transaction tx(db, Transaction::ReadWrite);
        base = start_base + CHUNK_SIZE - size1M * 1.5;
        allocator1.free((void *)base, size1M);
        tx.commit();
        base += size1M * 0.5;
    }
    {
        printf("Now allocate again, but just the smaller portion\n");
        Transaction tx(db, Transaction::ReadWrite);
        addr = allocator1.alloc(size1M * 0.5);
        passfail(testnum++, base, (long)addr);
        tx.commit();
    }
#endif

    // Also, there will be no header for this one.
    base = start_addr + 0xa00000;
    hdr_size = 24;
    printf("Testing borderline alloc within header size\n");
    Transaction tx2(db, Transaction::ReadWrite);
    addr = allocator1.alloc(CHUNK_SIZE - hdr_size);
    passfail(testnum++, base, (long)addr);
    tx2.commit();

    base = start_base + 4*CHUNK_SIZE;
    printf("Testing divisible borderline alloc\n");
    Transaction tx3(db, Transaction::ReadWrite);
    addr = allocator1.alloc(3 * CHUNK_SIZE);
    passfail(testnum++, base, (long)addr);
    tx3.commit();

    // Now test the free code.
    printf("Testing free for large borderline code\n");
    Transaction tx4(db, Transaction::ReadWrite);
    allocator1.free((void *)base, 3 * CHUNK_SIZE);

    // Next large alloc smaller than the block just freed will
    // still come from the tail and not the free list
    base += 4*CHUNK_SIZE;
    printf("Next alloc won't use free list but the previous pages have not been returned yet\n");
    addr = allocator1.alloc(1.5 * CHUNK_SIZE);
    passfail(testnum++, base - size1M, (long)addr);
    tx4.commit();

    printf("But this one will\n");
    Transaction tx5(db, Transaction::ReadWrite);
    base = start_base + 6*CHUNK_SIZE;
    addr = allocator1.alloc(CHUNK_SIZE - 8);
    passfail(testnum++, base, (long)addr);
    tx5.commit();

    base = start_base + CHUNK_SIZE - size1M * 0.5;
    // Now test the free code.
    printf("Testing free for large allocs\n");
    Transaction tx6(db, Transaction::ReadWrite);
    allocator1.free((void *)base, 3 * CHUNK_SIZE + size1M);
    tx6.commit();

    printf("Large tests done....\n");
}

int AllocTest::var_allocator_test()
{
    try {
        Graph::Config config;
        config.allocator_region_size = 104857600;  // 100MB
        Graph db("varallocgraph", Graph::Create | Graph::NoMsync, &config);
        Allocator *allocator1 = Allocator::get_main_allocator(db);
        start_addr = allocator1->get_start_addr();

        // While testing, also hit the case when the pool limit reaches.
        // That works but did not keep the test case here.
        var_freeform_tests(db, *allocator1);

        var_fixed_tests(db, *allocator1);

        // Now test the large and borderline allocations
        var_large_tests(db, *allocator1);
    }
    catch (Exception e)
    {
        print_exception(e);
        return 1;
    }

    return r;
}

void AllocTest::passfail(long id, long expected, long actual)
{
    if (expected == actual) {
        std::cout << "Test " << id << ": passed\n";
        return;
    }

    std::cerr << "Test " << id << ": failed: ";
    std::cerr << "expected " << std::hex << expected << "; ";
    std::cerr << "got " << std::hex << actual << "\n";

    r = 1;
}

static std::ostream& operator<< (std::ostream &out, Exception& e)
{
    out << "[Exception] " << e.name << " at " << e.file << ":" << e.line;
    return out;
}
