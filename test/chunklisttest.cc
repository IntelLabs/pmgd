/**
 * @file   chunklisttest.cc
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

#include <stdlib.h>  // for rand
#include <iostream>
#include <string.h>

#include "pmgd.h"
#include "../src/ChunkList.h"
#include "../src/os.h"
#include "../src/Allocator.h"
#include "util.h"
#include "../src/KeyValuePair.h"
#include "../src/GraphConfig.h"

using namespace PMGD;
using namespace std;

namespace PMGD {
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
    uint64_t region_size, hdr_size;

    try {
        Graph db("chunklistgraph", Graph::Create);

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
