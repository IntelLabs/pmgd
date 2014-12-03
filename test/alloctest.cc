/*
 * This unit test exercises the fixed-size allocator.
 */

#include <iostream>
#include <string.h>
#include "jarvis.h"
#include "util.h"
#include "../src/allocator.h"
#include "../src/os.h"
#include "exception.h"

using namespace Jarvis;

static void passfail(long id, long expected, long actual);
static std::ostream& operator<< (std::ostream &out, Exception& e);

int main(int argc, char **argv)
{
    std::cout << "Fixed-size allocator unit test\n\n";
    uint64_t start_addr;
    uint64_t pool_size;
    unsigned object_size;

    try {
        Graph db("alloctestdummy", Graph::Create);
        Transaction tx(db, Transaction::ReadWrite);

        bool create1 = true;
        start_addr = 0x100000000;
        pool_size = 1024;
        object_size = 32;
        os::MapRegion region1(".", "region1", start_addr, pool_size, create1, create1, false);
        FixedAllocator allocator1(start_addr, object_size, pool_size, create1);
        long base1 = start_addr + /* sizeof(struct RegionHeader) */64;
        
        void *addr1 = allocator1.alloc();
        passfail(1, base1, (long)addr1);

        void *addr2 = allocator1.alloc();
        passfail(2, base1 + object_size, (long)addr2);

        allocator1.free(addr1);

        allocator1.free(addr2);

        addr2 = allocator1.alloc();
        passfail(3, base1 + object_size, (long)addr2);

        addr1 = allocator1.alloc();
        passfail(4, base1, (long)addr1);

        void *addr3 = allocator1.alloc();
        passfail(5, base1 + 2*object_size, (long)addr3);


        bool create2 = true;
        start_addr = 0x200000000;
        pool_size = 96;
        object_size = 32;
        os::MapRegion region2(".", "region2", start_addr, pool_size, create2, create2, false);
        FixedAllocator allocator2(start_addr, object_size, pool_size, create2);
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

        tx.commit();
    }
    catch (Exception e)
    {
        print_exception(e);
        return 1;
    }

    return 0;
}

static void passfail(long id, long expected, long actual)
{
    if (expected == actual) {
        std::cout << "Test " << id << ": passed\n";
        return;
    }

    std::cerr << "Test " << id << ": failed: ";
    std::cerr << "expected " << std::hex << expected << "; ";
    std::cerr << "got " << std::hex << actual << "\n";

    exit(1);
}

static std::ostream& operator<< (std::ostream &out, Exception& e)
{
    out << "[Exception] " << e.name << " at " << e.file << ":" << e.line;
    return out;
}
