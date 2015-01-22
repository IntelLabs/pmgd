#include <iostream>
#include "jarvis.h"
#include "util.h"

using namespace Jarvis;

long long num_nodes = 0;
long long num_edges = 0;

static void node_added(Node &);
static void edge_added(Edge &);

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input>" << std::endl;
        return -1;
    }

    try {
        Graph db("load_gson_graph", Graph::Create);

        load_gson(db, argv[1], node_added, edge_added);
        Transaction tx(db);
        dump_nodes(db);
        dump_edges(db);
        tx.commit();
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    std::cout << "\nNodes (" << num_nodes << ")\t";
    std::cout << "Edges (" << num_edges << ")\n";

    return 0;
}

static void node_added(Node &n)
{
    ++num_nodes;
}

static void edge_added(Edge &e)
{
    ++num_edges;
}
