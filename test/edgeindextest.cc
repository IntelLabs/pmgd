#include <stdlib.h>  // for rand
#include <iostream>
#include <string.h>

#include "jarvis.h"
#include "../src/List.h"
#include "../src/os.h"
#include "../src/allocator.h"
#include "../src/EdgeIndex.h"
#include "../src/GraphConfig.h"
#include "util.h"

using namespace Jarvis;
using namespace std;

#define NUM_TEST_ELEMS 50

#define REGION_SIZE 8092
#define NUM_FIXED_ALLOCATORS 5

static constexpr AllocatorInfo default_allocators[] = {
    { 16, 0, REGION_SIZE },
    { 32, 1*REGION_SIZE, REGION_SIZE },
    { 64, 2*REGION_SIZE, REGION_SIZE },
    { 128, 3*REGION_SIZE, REGION_SIZE },
    { 256, 4*REGION_SIZE, REGION_SIZE },
};

int main()
{
    cout << "EdgeIndex unit test\n\n";
    uint64_t start_addr;
    uint64_t region_size;

    try {
        Graph db("edgeindexgraph", Graph::Create);

        Transaction tx(db, Transaction::ReadWrite);
        bool create1 = true;

        start_addr = 0x200000000;
        region_size = NUM_FIXED_ALLOCATORS * REGION_SIZE;

        os::MapRegion region1(".", "region1", start_addr, region_size, create1, create1, false);
        Allocator allocator1(start_addr, NUM_FIXED_ALLOCATORS, NULL, default_allocators, create1);

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
