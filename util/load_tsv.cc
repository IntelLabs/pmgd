#include <stdio.h>
#include <string.h>
#include "jarvis.h"
#include "util.h"

#undef Exception

using namespace Jarvis;

static const char ID_STR[] = "jarvis.loader.id";

static Node &get_node(Graph &db, long long id,
                      std::function<void(Node &)> node_func);

void load_tsv(Graph &db, const char *filename,
              std::function<void(Node &)> node_func,
              std::function<void(Edge &)> edge_func)
{
    FILE *f = strcmp(filename, "-") == 0 ? stdin : fopen(filename, "r");
    if (f == NULL)
        throw Jarvis::Exception(201, "load failed", errno, filename, __FILE__, __LINE__);

    load_tsv(db, f, node_func, edge_func);
}

void load_tsv(Graph &db, FILE *f,
              std::function<void(Node &)> node_func,
              std::function<void(Edge &)> edge_func)
{
    char buf[500];

    Transaction tx(db, Transaction::ReadWrite);
    db.create_index(Graph::NodeIndex, 0, ID_STR, PropertyType::t_integer);
    tx.commit();

    while (fgets(buf, sizeof buf, f) != NULL) {
        long long a, b;
        if (sscanf(buf, "%lld %lld", &a, &b) != 2)
            throw Jarvis::Exception(202, "load failed", "invalid input format", __FILE__, __LINE__);
        Transaction tx(db, Transaction::ReadWrite);
        Node &src = get_node(db, a, node_func);
        Node &dst = get_node(db, b, node_func);
        Edge &edge = db.add_edge(src, dst, 0);
        if (edge_func)
            edge_func(edge);
        tx.commit();
    }
}


static Node &get_node(Graph &db, long long id,
                      std::function<void(Node &)> node_func)
{
    NodeIterator nodes = db.get_nodes(0,
                             PropertyPredicate(ID_STR, PropertyPredicate::Eq, id));
    if (nodes) return *nodes;

    // Node not found; add it
    Node &node = db.add_node(0);
    node.set_property(ID_STR, id);
    if (node_func)
        node_func(node);
    return node;
}

void do_nothing_node(Jarvis::Node &) { }
void do_nothing_edge(Jarvis::Edge &) { }
