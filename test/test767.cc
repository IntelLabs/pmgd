/*
 * This test checks Mantis #767
 */

#include <stdio.h>
#include <stdlib.h>
#include "jarvis.h"
#include "../util/util.h"

using namespace Jarvis;

static int count(EdgeIterator &&);

#define CHECK(iter, v) { \
        int c = count(iter); \
        if (c == v) { \
            printf("count(%s) = %d\n", #iter, c); \
            dump(iter); \
        } \
        if (c != v) { \
            printf("count(%s) = %d, expected %d\n", #iter, c, v); \
            dump(iter); \
            r = EXIT_FAILURE; \
        } \
    }

int main(int argc, char **argv)
{
    try {
        int r = EXIT_SUCCESS;

        Graph db("test767graph", Graph::Create);

        Transaction tx(db, Transaction::ReadWrite);

        // Add three nodes
        Node &n1 = db.add_node(0);
        Node &n2 = db.add_node(0);
        Node &n3 = db.add_node(0);
        Node &n4 = db.add_node(0);

        db.add_edge(n1, n2, "tag1");

        CHECK(n1.get_edges(), 1);
        CHECK(n1.get_edges(Outgoing), 1);
        CHECK(n1.get_edges(Incoming), 0);

        CHECK(n1.get_edges("tag1"), 1);
        CHECK(n1.get_edges("tag2"), 0);
        CHECK(n1.get_edges(Outgoing, "tag1"), 1);
        CHECK(n1.get_edges(Outgoing, "tag2"), 0);
        CHECK(n1.get_edges(Incoming, "tag1"), 0);
        CHECK(n1.get_edges(Incoming, "tag2"), 0);

        db.add_edge(n3, n1, "tag2");

        CHECK(n1.get_edges(), 2);
        CHECK(n1.get_edges(Outgoing), 1);
        CHECK(n1.get_edges(Incoming), 1);

        CHECK(n1.get_edges("tag1"), 1);
        CHECK(n1.get_edges("tag2"), 1);
        CHECK(n1.get_edges(Outgoing, "tag1"), 1);
        CHECK(n1.get_edges(Outgoing, "tag2"), 0);
        CHECK(n1.get_edges(Incoming, "tag1"), 0);
        CHECK(n1.get_edges(Incoming, "tag2"), 1);

        db.add_edge(n1, n3, "tag1");
        db.add_edge(n4, n1, "tag1");
        db.add_edge(n1, n4, "tag2");

        CHECK(n1.get_edges(), 5);
        CHECK(n1.get_edges(Outgoing), 3);
        CHECK(n1.get_edges(Incoming), 2);

        CHECK(n1.get_edges("tag1"), 3);
        CHECK(n1.get_edges("tag2"), 2);
        CHECK(n1.get_edges(Outgoing, "tag1"), 2);
        CHECK(n1.get_edges(Outgoing, "tag2"), 1);
        CHECK(n1.get_edges(Incoming, "tag1"), 1);
        CHECK(n1.get_edges(Incoming, "tag2"), 1);

        return r;
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }
}

static int count(EdgeIterator &&i)
{
    int n = 0;
    i.process([&n](Edge&){n++;});
    return n;
}
