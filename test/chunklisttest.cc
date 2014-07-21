#include <stdlib.h>  // for rand
#include <iostream>
#include <string.h>

#include "jarvis.h"
#include "../src/ChunkList.h"
#include "../src/os.h"
#include "../src/allocator.h"
#include "../util/util.h"
#include "../src/KeyValuePair.h"

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

namespace Jarvis {
    class ChunkListTest {
    public:
        void print(ChunkList<int,int,64> *list)
        {
            ChunkList<int,int,64>::ChunkListType *curr = list->_head;
            // This list is not sorted, so search till the end
            while (curr != NULL) {  // across chunks
                printf("Bitmask: 0x%x\n", curr->occupants);
                // Within a chunk
                // Within a chunk
                int elems = 0, curr_slot = 0;
                uint16_t bit_pos = 1;
                while (elems < curr->num_elems) {
                    // Check if there is a 1 at the bit position being tested.
                    // If the value is non-zero, there is, else not
                    int elem_idx = curr->occupants & bit_pos;
                    if (elem_idx != 0) { // non-empty slot
                        cout << "curr->key[" << curr_slot << "]: " << curr->data[curr_slot].key()
                            << ", curr->value: " << curr->data[curr_slot].value() << "\n";
                        elems++;
                    }
                    bit_pos <<= 1;
                    // Keep current with index position
                    ++curr_slot;
                }
                // If not found in this chunk, move to next
                curr = curr->next;
            }
        }
    };
}

int main()
{
    cout << "ChunkList unit test\n\n";
    uint64_t start_addr;

    try {
        Graph db("chunklistgraph", Graph::Create);

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

        os::MapRegion region1(".", "region1", start_addr, info1.len, create1, create1, false);
        Allocator allocator1(start_addr, info_arr, NUM_FIXED_ALLOCATORS, create1);

        ChunkList<int,int,64> *list = (ChunkList<int,int,64> *)allocator1.alloc(sizeof *list);
        ChunkListTest test;
        cout << "Size of chunk list obj: " << sizeof(ChunkList<int,int,64>) << "\n";
        list->init();

        // ChunkList array elements
        int elems[] = {7, 5, 3, 7, 4, 13, 3, 89, 70, 100, 12, 15, 41, 56, 80, 95, 14445};
        for (int i = 0; i < 5; ++i) {
            int *value = list->add(elems[i], allocator1);
            *value = i;
        }
        cout << "Number of elems after adding 7,5,3,7,4: " << list->num_elems() << "\n";
        test.print(list);
        /*
           for (int i = 0; i < 100; ++i)
           list.add(rand() % 1000);
           */
        list->remove(3, allocator1);
        for (int i = 5; i < 17; ++i) {
            int *value = list->add(elems[i], allocator1);
            *value = i;
        }
        cout << "Number of elems after removing 3, and adding the rest: " << list->num_elems() << "\n";
        test.print(list);
        list->remove(14445, allocator1);
        list->remove(95, allocator1);
        list->remove(80, allocator1);
        cout << "Number of elems after removing last 3: " << list->num_elems() << "\n";
        test.print(list);

        for (int i = 0; i < 17; ++i)
            list->remove(elems[i], allocator1);
        cout << "Number of elems after removing: " << list->num_elems() << "\n";
        test.print(list);
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }
    return 0;
}
