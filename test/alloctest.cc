/*
 * This unit test exercises the fixed-size allocator.
 */

#include <iostream>
#include <string.h>
#include "../src/allocator.h"
#include "../include/exception.h"

using namespace Jarvis;

static void passfail(long id, long expected, long actual);
static std::ostream& operator<< (std::ostream &out, Exception& e);

int main(int argc, char **argv)
{
    std::cout << "Fixed-size allocator unit test\n\n";

    struct AllocatorInfo info1;
    strcpy(info1.name, "region1");
    info1.addr = 0x100000000;
    info1.len = 1024;
    info1.size = 32;
    FixedAllocator region1(".", info1, true);
    long base1 = info1.addr + /* sizeof(struct RegionHeader) */64;
    
    void *addr1 = region1.alloc();
    passfail(1, base1, (long)addr1);

    void *addr2 = region1.alloc();
    passfail(2, base1 + info1.size, (long)addr2);

    region1.free(addr1);

    region1.free(addr2);

    addr2 = region1.alloc();
    passfail(3, base1 + info1.size, (long)addr2);

    addr1 = region1.alloc();
    passfail(4, base1, (long)addr1);

    void *addr3 = region1.alloc();
    passfail(5, base1 + 2*info1.size, (long)addr3);


    struct AllocatorInfo info2;
    strcpy(info2.name, "region2");
    info2.addr = 0x200000000;
    info2.len = 96;
    info2.size = 32;
    FixedAllocator region2(".", info2, true);
    long base2 = info2.addr + /* sizeof(struct RegionHeader) */64;

    addr1 = region2.alloc();
    passfail(6, base2, (long)addr1);

    bool ok = false;
    try {
        addr2 = region2.alloc();
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
