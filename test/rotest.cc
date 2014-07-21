/**
 * Dump the content of a graphstore to standard output
 */

#include <string.h>
#include <stdio.h>
#include "jarvis.h"
#include "../util/util.h"

using namespace Jarvis;

#undef Exception

void create_db(const char *name)
{
    Graph db(name, Graph::Create);
}

// Test that a read-only operation works.
void test1(Graph &db)
{
    Transaction tx(db);
    dump_nodes(db);
}

// Test that a read-only transaction cannot be used to modify the db.
void test2(Graph &db)
{
    Transaction tx(db);
    try {
        db.add_node(0);
    }
    catch (Exception e) {
        if (e.num != Exception::e_read_only)
            throw e;
        return;
    }
    throw Exception(111, "unexpected", __FILE__, __LINE__);
}

// Test that a read-write transaction cannot be created.
void test3(Graph &db)
{
    try {
        Transaction tx(db, Transaction::ReadWrite);
    }
    catch (Exception e) {
        if (e.num != Exception::e_read_only)
            throw e;
        return;
    }
    throw Exception(111, "unexpected", __FILE__, __LINE__);
}


int main(int argc, char **argv)
{
    const char name[] = "rograph";

    try {
        create_db(name);

        Graph db(name, Graph::ReadOnly);

        test1(db);
        test2(db);
        test3(db);
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
