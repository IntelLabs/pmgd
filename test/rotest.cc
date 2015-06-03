/**
 * Dump the content of a graphstore to standard output
 */

#include <string.h>
#include <stdio.h>
#include "jarvis.h"
#include "util.h"

using namespace Jarvis;

#undef Exception

void create_db(const char *name)
{
    Graph db(name, Graph::Create);
    Transaction tx(db, Transaction::ReadWrite);
    db.add_node(0);
    db.add_node("");
    db.add_node("x");
}

// Test that read-only operations work.
void test1(Graph &db)
{
    Transaction tx(db);
    dump_nodes(db);
    StringID id1(0);
    StringID id2("");
    StringID id3("x");
    StringID id;
    if (!StringID::lookup("x", id))
        throw Exception(111, "unexpected", __FILE__, __LINE__);
    if (StringID::lookup("y", id))
        throw Exception(111, "unexpected", __FILE__, __LINE__);
}

// Test that a read-only transaction cannot be used to modify the db.
void test2a(Graph &db)
{
    try {
        db.add_node(0);
    }
    catch (Exception e) {
        if (e.num != Exception::ReadOnly)
            throw e;
        return;
    }
    throw Exception(111, "unexpected", __FILE__, __LINE__);
}

void test2b(Graph &db)
{
    try {
        // Try to create a stringid that doesn't exist
        StringID id("y");
    }
    catch (Exception e) {
        if (e.num != Exception::ReadOnly)
            throw e;
        return;
    }
    throw Exception(111, "unexpected", __FILE__, __LINE__);
}

void test2(Graph &db)
{
    Transaction tx(db);
    test2a(db);
    test2b(db);
}


// Test that a read-write transaction cannot be created in a read-only graph.
void test3(Graph &db)
{
    try {
        Transaction tx(db, Transaction::ReadWrite);
    }
    catch (Exception e) {
        if (e.num != Exception::ReadOnly)
            throw e;
        return;
    }
    throw Exception(111, "unexpected", __FILE__, __LINE__);
}


// Test that a read-only nested transaction is allowed.
void test4a(Graph &db)
{
    Transaction tx(db);
}

// Test that the outer transaction is still usable.
void test4b(Graph &db)
{
    db.add_node(0);
}

// Test that a nested read-write transaction cannot be created.
void test4c(Graph &db)
{
    try {
        Transaction tx(db, Transaction::ReadWrite);
    }
    catch (Exception e) {
        if (e.num != Exception::NotImplemented)
            throw e;
        return;
    }
    throw Exception(111, "unexpected", __FILE__, __LINE__);
}

// Test that the outer transaction is still usable.
void test4d(Graph &db)
{
    db.add_node(0);
}


// Test mixing read-only and read-write transactions (in a read-write graph).
void test4(Graph &db)
{
    Transaction tx(db, Transaction::ReadWrite);
    db.add_node(0);
    test4a(db);
    test4b(db);
    test4c(db);
    test4d(db);
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

    try {
        Graph db(name);
        test4(db);
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
