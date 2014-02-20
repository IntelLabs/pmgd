#include <stdlib.h>  // for rand
#include <iostream>
#include <string.h>
#include "EdgeIndex.h"
#include "allocator.h"

using namespace Jarvis;
using namespace std;

#define NUM_TEST_ELEMS 50

int main()
{
    cout << "EdgeIndex unit test\n\n";

    // Need the allocator
    struct AllocatorInfo info1;
    strcpy(info1.name, "region1");
    info1.addr = 0x200000000;
    info1.len = 8192;
    info1.size = 64;
    FixedAllocator region1(".", info1, true);
    long base1 = info1.addr + /* sizeof(struct RegionHeader) */64;

    cout << "Allocator created starting at: " << (void *)base1 << "\n";

    cout << "Step 1: Test pair\n";
    KeyValuePair<Edge *, Node *> entry1((Edge *)region1.alloc(), (Node *)region1.alloc());
    cout << "Elem1 in entry1: " << entry1.key() << ", e2: " << entry1.value() << "\n";
    KeyValuePair<Edge *, Node *> entry2((Edge *)region1.alloc(), (Node *)region1.alloc());
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
    KeyValuePair<Edge *, Node *> entry3((Edge *)region1.alloc(), (Node *)region1.alloc());
    cout << "Elem1 in entry3: " << entry3.key() << ", e2: " << entry3.value() << "\n";
    cout << endl;

#ifdef TYPE_OUTSIDE
    cout << "Step 2: Test EdgeIndexType\n";
    EdgeIndexType idx(20);
    try {
        idx.add(entry1);
    } catch (Exception e_bad_alloc) {
        cout << "Add to empty list\n";           
    }
    // These won't work if the class is moved back into the main 
    // index class
    EdgeIndexType newkey(20, (Edge *)region1, (Node *)region1.alloc(), entry1);
    newkey.print();
    newkey.add(entry3);
    newkey.print();
#endif

    cout << "Step 3: Testing the index\n";
    EdgeIndex *edge_table = EdgeIndex::create(region1);

    edge_table->add(20, entry1.key(), entry1.value(), region1);

    edge_table->add(20, entry3.key(), entry3.value(), region1);
    edge_table->add(10, entry2.key(), entry2.value(), region1);

    edge_table->remove(10, entry2.key(), entry2.value(), region1);
    edge_table->remove(20, entry3.key(), entry3.value(), region1);
    edge_table->remove(20, entry1.key(), entry1.value(), region1);
    edge_table->add(20, entry2.key(), entry2.value(), region1);

    return 0;
}
