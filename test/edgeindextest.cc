#include <stdlib.h>  // for rand
#include <iostream>
#include <string.h>

#include "jarvis.h"
#include "../src/List.h"
#include "../src/os.h"
#include "../src/allocator.h"
#include "../src/EdgeIndex.h"
#include "../util/util.h"

using namespace Jarvis;
using namespace std;

#define NUM_TEST_ELEMS 50

#define REGION_SIZE 8092
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
    cout << "EdgeIndex unit test\n\n";
    uint64_t start_addr;

    try {
        Graph db("edgeindexgraph", Graph::Create);

        Transaction tx(db);
        // Need the allocator
        struct AllocatorInfo info1;
        struct AllocatorInfo info_arr[sizeof default_allocators];
        bool create1 = true;

        start_addr = 0x200000000;
        memcpy(info_arr, default_allocators, sizeof default_allocators);

        info1.offset = 0;
        info1.len = NUM_FIXED_ALLOCATORS * REGION_SIZE;
        info1.size = 0;

        os::MapRegion region1(".", "region1", start_addr, info1.len, create1, create1);
        Allocator allocator1(start_addr, info_arr, NUM_FIXED_ALLOCATORS, create1);

        cout << "Step 1: Test pair\n";
        // Since we do not have an allocator for anything < 16B, create large ptrs
        KeyValuePair<Edge *, Node *> entry1((Edge *)allocator1.alloc(16),
                (Node *)allocator1.alloc(16));
        cout << "Elem1 in entry1: " << entry1.key() << ", e2: " << entry1.value() << "\n";
        KeyValuePair<Edge *, Node *> entry2((Edge *)allocator1.alloc(16),
                (Node *)allocator1.alloc(16));
        cout << "Elem1 in entry2: " << entry2.key() << ", e2: " << entry2.value() << "\n";

        if (entry1 == entry2) 
            cout << "entry1 = entry2\n";
        else if (entry1 < entry2)
            cout << "entry1 < entry2\n";
        else
            cout << "entry1 > entry2\n";

        entry2.set_key(entry1.key());
        if (entry1 == entry2) 
            cout << "entry1 = entry2\n";
        else if (entry1 < entry2)
            cout << "entry1 < entry2\n";
        else
            cout << "entry1 > entry2\n";
        KeyValuePair<Edge *, Node *> entry3((Edge *)allocator1.alloc(16),
                (Node *)allocator1.alloc(16));
        cout << "Elem1 in entry3: " << entry3.key() << ", e2: " << entry3.value() << "\n";
        cout << endl;

        cout << "Step 3: Testing the index\n";
        EdgeIndex *edge_table = EdgeIndex::create(allocator1);

        edge_table->add("tag20", entry1.key(), entry1.value(), allocator1);

        edge_table->add("tag20", entry3.key(), entry3.value(), allocator1);
        edge_table->add("tag10", entry2.key(), entry2.value(), allocator1);

        edge_table->remove("tag10", entry2.key(), entry2.value(), allocator1);
        edge_table->remove("tag20", entry3.key(), entry3.value(), allocator1);
        edge_table->remove("tag20", entry1.key(), entry1.value(), allocator1);
        edge_table->add("tag20", entry2.key(), entry2.value(), allocator1);
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
