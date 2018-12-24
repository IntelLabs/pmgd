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
namespace PMGD {
    class EdgeIndexTest {
        int run_edge_test(Graph &db, Allocator &allocator1);
    public:
        int run_test();
    };
}

int main()
{
    cout << "EdgeIndex unit test\n\n";
    EdgeIndexTest ett;
    return ett.run_test();
}

int EdgeIndexTest::run_edge_test(Graph &db, Allocator &allocator1)
{
    Transaction tx(db, Transaction::ReadWrite);
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

    tx.commit();

    return 0;
}

int EdgeIndexTest::run_test()
{
    try {
        Graph db("edgeindexgraph", Graph::Create);
        Allocator *allocator1 = Allocator::get_main_allocator(db);

        run_edge_test(db, *allocator1);
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
