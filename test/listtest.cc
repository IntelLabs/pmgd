#include <stdlib.h>  // for rand
#include <iostream>
#include <string.h>

#include "jarvis.h"
#include "../src/List.h"
#include "../src/os.h"
#include "../src/Allocator.h"
#include "../src/GraphConfig.h"
#include "util.h"

using namespace Jarvis;
using namespace std;

int main()
{
    cout << "List unit test\n\n";
    uint64_t start_addr;
    uint64_t region_size, hdr_size;

    try {
        Graph db("listgraph", Graph::Create);

        Transaction tx(db, Transaction::ReadWrite);
        bool create1 = true;

        start_addr = 0x100000000;
        region_size = 10485760;       // 10MB
        // We need a PM space for allocator header which normally
        // will reside in the GraphInfo structure which is quite
        // hidden. So creating a temporary space here to allow for
        // the header.
        uint64_t hdr_addr = start_addr + region_size;
        hdr_size = 1024;
        os::MapRegion region1(".", "region1", start_addr, region_size, create1, create1, false);
        os::MapRegion region2(".", "region2", hdr_addr, hdr_size, create1, create1, false);
        Allocator::RegionHeader *hdr = reinterpret_cast<Allocator::RegionHeader *>(hdr_addr);
        Allocator allocator1(start_addr, region_size, hdr, create1);

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
