#include <iostream>
#include "jarvis.h"
#include "util.h"

using namespace Jarvis;

long long num_nodes = 0;
long long num_edges = 0;

void node_added(Node &);
void edge_added(Edge &);

int main(int argc, char *argv[])
{
    long line = 0;

    try {
        Graph db("load_tsv_graph", Graph::Create);

        load_tsv(db, stdin, node_added, edge_added);
    }
    catch (Exception e) {
        std::cerr << argv[0] << ": stdin: Exception " << e.name << " occurred on line " << line << "\n";
        return 1;
    }

    std::cout << num_nodes << "\t" << num_edges << "\n";

    return 0;
}

void node_added(Node &n)
{
    ++num_nodes;
}

void edge_added(Edge &e)
{
    ++num_edges;
}
