#include <stdio.h>
#include "jarvis.h"
#include "../util/util.h"

#undef Exception

using namespace Jarvis;

#define ID "id"

static Node &get_node(Graph &db, long long id);

void load(Graph &db, const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (f == NULL)
        throw Jarvis::Exception(201, "load_failed", __FILE__, __LINE__);

    load(db, f);
}

void load(Graph &db, FILE *f)
{
    char buf[500];
    while (fgets(buf, sizeof buf, f) != NULL) {
        long long a, b;
        if (sscanf(buf, "%lld %lld", &a, &b) != 2)
            throw Jarvis::Exception(201, "load_failed", __FILE__, __LINE__);
        Transaction tx(db);
        Node &src = get_node(db, a);
        Node &dst = get_node(db, b);
        db.add_edge(src, dst, 0);
        tx.commit();
    }
}


static Node &get_node(Graph &db, long long id)
{
#if 0
    // This API not available yet
    NodeIterator nodes = db.get_nodes(0,
                             PropertyPredicate(ID, PropertyPredicate::eq, id));
    if (nodes) return *nodes;
#else
    NodeIterator i = db.get_nodes();

    NodeIterator j = i.filter([id](const Node &n)
                              { return n.get_property(ID).int_value()
                                == id ? pass : dont_pass; });
    if (j) return *j;
#endif

    // Node not found; add it
    Node &node = db.add_node(0);
    node.set_property(ID, id);
    return node;
}
