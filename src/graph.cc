#include "graph.h"
#include "node.h"
#include "edge.h"
#include "allocator.h"
#include "iterator.h"
#include "os.h"

class Jarvis::Graph::GraphImpl {
    typedef FixedAllocator NodeTable;
    typedef FixedAllocator EdgeTable;

    static const size_t BASE_ADDRESS = 0x10000000000;
    static const size_t REGION_SIZE = 0x10000000000;
    static const unsigned INDEX_SIZE = 4096;
    static const unsigned NODE_SIZE = 64;
    static const unsigned EDGE_SIZE = 32;

    static constexpr char index_name[] = "index.jdb";

    static constexpr AllocatorInfo default_allocators[] = {
        { "nodes.jdb", BASE_ADDRESS + REGION_SIZE, REGION_SIZE, NODE_SIZE },
        { "edges.jdb", BASE_ADDRESS + 2*REGION_SIZE, REGION_SIZE, EDGE_SIZE },
    };

    struct GraphIndex {
        uint64_t version;

        // node_table, edge_table, property_chunks
        AllocatorInfo node_info;
        AllocatorInfo edge_info;

        // Transaction info
        // Lock table info
    };

    class GraphInit {
        bool _create;
        os::MapRegion _index_map;
        GraphIndex *_index;

    public:
        GraphInit(const char *name, int options);
        bool create() { return _create; }
        const AllocatorInfo &node_info() { return _index->node_info; }
        const AllocatorInfo &edge_info() { return _index->edge_info; }
    };

    // ** Order here is important: GraphInit MUST be first
    GraphInit _init;

    NodeTable _node_table;
    EdgeTable _edge_table;
    // Other Fixed ones
    // Variable allocator

    // Transactions
    // Lock manager

public:
    GraphImpl(const char *name, int options)
        : _init(name, options),
          _node_table(name, _init.node_info(), _init.create()),
          _edge_table(name, _init.edge_info(), _init.create())
        { }
    NodeTable &node_table() { return _node_table; }
    EdgeTable &edge_table() { return _edge_table; }
};

Jarvis::Graph::Graph(const char *name, int options)
    : _impl(new GraphImpl(name, options))
{
}

Jarvis::Graph::~Graph()
{
    delete _impl;
}

Jarvis::Node &Jarvis::Graph::add_node(StringID tag)
{
    Node *node = (Node *)_impl->node_table().alloc();
    node->init(tag);
    return *node;
}

Jarvis::Edge &Jarvis::Graph::add_edge(Node &src, Node &dest, StringID tag)
{
    Edge *edge = (Edge *)_impl->edge_table().alloc();
    edge->init(src, dest, tag);
    return *edge;
}


constexpr char Jarvis::Graph::GraphImpl::index_name[];
constexpr Jarvis::AllocatorInfo Jarvis::Graph::GraphImpl::default_allocators[];

Jarvis::Graph::GraphImpl::GraphInit::GraphInit(const char *name, int options)
    : _create(options & Create),
      _index_map(name, index_name, BASE_ADDRESS, INDEX_SIZE, _create, false),
      _index(reinterpret_cast<GraphIndex *>(BASE_ADDRESS))
{
    // _create was modified by _index_map constructor
    // depending on whether the file existed or not

    // Set up the info structure
    if (_create) {
        // Version info
        _index->version = 1;

        // TODO replace static indexing
        _index->node_info = default_allocators[0];
        _index->edge_info = default_allocators[1];

        // Other information
    }
}

namespace Jarvis {
    class Graph_NodeIterator : public NodeIteratorImpl {
        const FixedAllocator &node_table;
        void *current_node;
        void _next();
        void _skip();

    public:
        Graph_NodeIterator(const FixedAllocator &);
        operator bool() const { return current_node != NULL; }
        Node &operator*() const { return *(Node *)current_node; }
        Node *operator->() const { return (Node *)current_node; }
        void next();
    };
};

Jarvis::Graph_NodeIterator::Graph_NodeIterator(const FixedAllocator &n)
    : node_table(n)
{
    current_node = node_table.begin();
    _next();
}

void Jarvis::Graph_NodeIterator::next()
{
    _skip();
    _next();
}

void Jarvis::Graph_NodeIterator::_next()
{
    while (current_node < node_table.end() && node_table.is_free(current_node))
        _skip();

    if (current_node >= node_table.end())
        current_node = NULL;
}

void Jarvis::Graph_NodeIterator::_skip()
{
    current_node = node_table.next(current_node);
}

Jarvis::NodeIterator Jarvis::Graph::get_nodes()
{
    return NodeIterator(new Graph_NodeIterator(_impl->node_table()));
}
