#include <stdlib.h>  // for rand
#include <iostream>
#include <string.h>

#include "jarvis.h"
#include "../src/List.h"
#include "../src/os.h"
#include "../src/allocator.h"
#include "../src/GraphConfig.h"
#include "util.h"

using namespace Jarvis;
using namespace std;

#define REGION_SIZE 4096
#define NUM_FIXED_ALLOCATORS 5

static const AllocatorInfo default_allocators[] = {
    { 16, 0, REGION_SIZE },
    { 32, 1*REGION_SIZE, REGION_SIZE },
    { 64, 2*REGION_SIZE, REGION_SIZE },
    { 128, 3*REGION_SIZE, REGION_SIZE },
    { 256, 4*REGION_SIZE, REGION_SIZE },
};

int main()
{
    cout << "List unit test\n\n";
    uint64_t start_addr;
    uint64_t region_size;

    try {
        Graph db("listgraph", Graph::Create);

        Transaction tx(db, Transaction::ReadWrite);
        bool create1 = true;

        start_addr = 0x100000000;
        region_size = NUM_FIXED_ALLOCATORS * REGION_SIZE;

        os::MapRegion region1(".", "region1", start_addr, region_size, create1, create1, false);
        Allocator allocator1(start_addr, NUM_FIXED_ALLOCATORS, NULL, default_allocators, create1);

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
