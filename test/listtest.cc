#include <stdlib.h>  // for rand
#include <iostream>
#include <string.h>

#include "jarvis.h"
#include "../src/List.h"
#include "../src/os.h"
#include "../src/allocator.h"
#include "../util/util.h"

using namespace Jarvis;
using namespace std;

#define REGION_SIZE 4096
#define NUM_FIXED_ALLOCATORS 5

static constexpr AllocatorInfo default_allocators[] = {
    { 0, REGION_SIZE, 16 },
    { 1*REGION_SIZE, REGION_SIZE, 32 },
    { 2*REGION_SIZE, REGION_SIZE, 64 },
    { 3*REGION_SIZE, REGION_SIZE, 128 },
    { 4*REGION_SIZE, REGION_SIZE, 256 },
};

int main()
{
    cout << "List unit test\n\n";
    uint64_t start_addr;

    try {
        Graph db("listgraph", Graph::Create);

        Transaction tx(db, Transaction::ReadWrite);
        // Need the allocator
        struct AllocatorInfo info1;
        struct AllocatorInfo info_arr[sizeof default_allocators];
        bool create1 = true;

        start_addr = 0x100000000;
        memcpy(info_arr, default_allocators, sizeof default_allocators);

        info1.offset = 0;
        info1.len = NUM_FIXED_ALLOCATORS * REGION_SIZE;
        info1.size = 0;

        os::MapRegion region1(".", "region1", start_addr, info1.len, create1, create1);
        Allocator allocator1(start_addr, info_arr, NUM_FIXED_ALLOCATORS, create1);

        List<int> *list = (List<int> *)allocator1.alloc(sizeof *list);
        cout << "Size of list obj: " << sizeof(List<int>) << "\n";
        list->init();

        // List array elements
        int elems[] = {7, 5, 3, 7, 4, 13, 3};
        for (int i = 0; i < 5; ++i)
            list->add(elems[i], allocator1);
        cout << "Number of elems after adding 7,5,3,7,4: " << list->num_elems() << "\n";
        /*
           for (int i = 0; i < 100; ++i)
           list.add(rand() % 1000);
           */
        list->remove(3, allocator1);
        for (int i = 5; i < 7; ++i)
            list->add(elems[i], allocator1);
        cout << "Number of elems after removing 3, and adding 13, 3: " << list->num_elems() << "\n";

        for (int i = 0; i < 7; ++i)
            list->remove(elems[i], allocator1);
        cout << "Number of elems after removing: " << list->num_elems() << "\n";
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }
    return 0;
}
