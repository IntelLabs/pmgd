/**
 * @file   edgeindextest.cc
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
#include "../src/List.h"
#include "../src/os.h"
#include "../src/Allocator.h"
#include "../src/EdgeIndex.h"
#include "../src/GraphConfig.h"
#include "util.h"

using namespace PMGD;
using namespace std;

#define NUM_TEST_ELEMS 50

int main()
{
    cout << "EdgeIndex unit test\n\n";
    uint64_t start_addr;
    uint64_t region_size, hdr_size;

    try {
        Graph db("edgeindexgraph", Graph::Create);

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

        edge_table->remove("tag10", entry2.key(), allocator1);
        edge_table->remove("tag20", entry3.key(), allocator1);
        edge_table->remove("tag20", entry1.key(), allocator1);
        edge_table->add("tag20", entry2.key(), entry2.value(), allocator1);
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
