/**
 * @file   txtest.cc
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
 * Test for PMGD transactions
 */

#include <stdio.h>
#include "pmgd.h"
#include "util.h"

using namespace PMGD;

static void dump(Graph &db);
static void dump_no_tx(Graph &db);
static void modify(Graph &db, int argc, char **argv, bool commit);
static void modify_nested(Graph &db, int argc, char **argv);

int main(int argc, char **argv)
{
    bool create = (argc > 1);

    try {
        // create graph outside transactions
        Graph db("txgraph", create ? Graph::Create : Graph::ReadOnly);
        modify(db, argc, argv, true);
        modify(db, argc, argv, false);
        modify_nested(db, argc, argv);
    }
    catch (Exception e) {
        print_exception(e);
        exit(1);
    }
    return 0;
}

static void modify_nested(Graph &db, int argc, char **argv)
{
    try {
        // Test nested independent transactions
        Transaction tx(db, Transaction::ReadWrite);
        modify(db, argc, argv, true);
        modify(db, argc, argv, false);
        tx.commit();
    }
    catch (Exception e) {
        print_exception(e);
        exit(1);
    }
}

static void modify(Graph &db, int argc, char **argv, bool commit)
{
    if (commit)
        printf("\nCOMMIT TEST\n");
    else
        printf("\nABORT TEST\n");

    try {
        // add nodes and edges in an independent transaction
        Transaction tx(db, Transaction::ReadWrite | Transaction::Independent);

        Node *prev = 0;
        for (int i = 1; i < argc; i++) {
            Node &n = db.add_node(0);
            n.set_property(0, argv[i]);
            if (prev != NULL)
                db.add_edge(*prev, n, 0);
            prev = &n;
        }

        if (commit) {
            tx.commit();
        }
        else {
            printf("BEFORE ABORT:\n");
            dump_no_tx(db);
        }
    }
    catch (Exception e) {
        print_exception(e);
        exit(1);
    }
    if (commit)
        printf("AFTER COMMIT:\n");
    else
        printf("AFTER ABORT:\n");
    dump(db);
}

static void dump_no_tx(Graph &db)
{
    try {
        dump_debug(db);
    }
    catch (Exception e) {
        print_exception(e);
        exit(1);
    }
}

static void dump(Graph &db)
{
    try {
        Transaction tx(db, Transaction::Independent);
        dump_debug(db);
        tx.commit();
    }
    catch (Exception e) {
        print_exception(e);
        exit(1);
    }
}
