/**
 * @file   listtest.cc
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
#include "../src/GraphConfig.h"
#include "util.h"

using namespace PMGD;
using namespace std;

namespace PMGD {
    class ListTest {
        List<int> *list;

    public:
        int run_test();
    };
}

int main()
{
    cout << "List unit test\n\n";
    ListTest lt;
    return lt.run_test();
}

int ListTest::run_test()
{
    try {
        Graph db("listgraph", Graph::Create);
        Allocator *allocator1 = Allocator::get_main_allocator(db);

        Transaction tx1(db, Transaction::ReadWrite);
        list = (List<int> *)allocator1->alloc(sizeof *list);
        cout << "Size of list obj: " << sizeof(List<int>) << "\n";
        list->init();

        // List array elements
        int elems[] = {7, 5, 3, 7, 4, 13, 3};
        for (int i = 0; i < 5; ++i)
            list->add(elems[i], *allocator1);
        cout << "Number of elems after adding 7,5,3,7,4: " << list->num_elems() << "\n";
        /*
           for (int i = 0; i < 100; ++i)
           list.add(rand() % 1000);
           */
        list->remove(3, *allocator1);
        for (int i = 5; i < 7; ++i)
            list->add(elems[i], *allocator1);
        cout << "Number of elems after removing 3, and adding 13, 3: " << list->num_elems() << "\n";

        for (int i = 0; i < 7; ++i)
            list->remove(elems[i], *allocator1);
        cout << "Number of elems after removing: " << list->num_elems() << "\n";
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }
    return 0;
}
