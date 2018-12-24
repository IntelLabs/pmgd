/**
 * @file   propertypredicatetest.cc
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
 * This test checks PMGD iterator filters
 */

#include <stdio.h>
#include "pmgd.h"
#include "util.h"

using namespace PMGD;

int main(int argc, char **argv)
{
    bool create = (argc > 1);

    try {
        Graph db("ppgraph", create ? Graph::Create : Graph::ReadOnly);

        Transaction tx(db, create ? Transaction::ReadWrite : 0);

        for (int i = 1; i < argc; i++) {
            char tag[14] = {0};
            sprintf(tag, "tag%d",i);
            Node &n = db.add_node(tag);
            n.set_property("id", argv[i]);
            n.set_property("value", i);
        }

        // Look for name starting with 'a'
        printf("Nodes starting with 'a'\n");
        dump(db.get_nodes()
            .filter(PropertyPredicate("id", PropertyPredicate::GeLt, "a", "b")));

        // Look for name starting with 'b'
        printf("Nodes starting with 'b'\n");
        dump(db.get_nodes()
            .filter(PropertyPredicate("id", PropertyPredicate::GeLt, "b", "c")));

        // Look for value less than 2
        printf("Nodes with value less than 2\n");
        dump(db.get_nodes()
            .filter(PropertyPredicate("value", PropertyPredicate::Lt, 2)));

        // Look for value less or equal to than 4
        printf("Nodes with value less than or equal to 4\n");
        dump(db.get_nodes()
            .filter(PropertyPredicate("value", PropertyPredicate::Le, 4)));

        // Look for value greater than 3
        printf("Nodes with value greater than 3\n");
        dump(db.get_nodes()
            .filter(PropertyPredicate("value", PropertyPredicate::Gt, 3)));

        // Look for value greater than or equal to  1
        printf("Nodes with value greater than or equal to 1\n");
        dump(db.get_nodes()
            .filter(PropertyPredicate("value", PropertyPredicate::Ge, 1)));

        // Look for value between 2 and 5
        printf("Nodes with value between 2 and 5\n");
        dump(db.get_nodes()
            .filter(PropertyPredicate("value", PropertyPredicate::GeLe, 2, 5)));

        tx.commit();
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
