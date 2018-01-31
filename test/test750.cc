/**
 * @file   test750.cc
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

/*
 * This test checks Mantis #750
 */

#include <stdio.h>
#include "pmgd.h"
#include "../util/util.h"

using namespace PMGD;

int main(int argc, char **argv)
{
    try {
        Graph db("test750graph", Graph::Create);

        switch (atoi(argv[1])) {
            case 1: {
                // Add nodes in first transaction
                Transaction tx(db, Transaction::ReadWrite);
                Node &n1 = db.add_node(0);
                n1.set_property("id", 1);
                Node &n2 = db.add_node(0);
                n2.set_property("id", 2);
                tx.commit();
                break;
            }

            case 2: {
                // Add property in second transaction
                Transaction tx(db, Transaction::ReadWrite);
                NodeIterator ni = db.get_nodes();
                Node &n1 = *ni;
                n1.set_property("a", 5);
                tx.commit();
                break;
            }

            case 3: {
                // Add property in third transaction
                // and then abort the transaction.
                Transaction tx(db, Transaction::ReadWrite);
                NodeIterator ni = db.get_nodes();
                Node &n1 = *ni;
                n1.set_property("abort", 2);
                exit(0);
            }

            case 4: {
                // Check that abort property is not present
                Transaction tx(db, Transaction::ReadOnly);
                NodeIterator ni = db.get_nodes();
                Node &n1 = *ni;
                printf("id = %lld\n", n1.get_property("id").int_value());
                Property p;
                if (n1.check_property("abort", p))
                    return 1;
                break;
            }
        }
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
