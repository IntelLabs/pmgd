#include <stddef.h>
#include <string.h>
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
    static const unsigned INFO_SIZE = 4096;
    static const unsigned NODE_SIZE = 64;
    static const unsigned EDGE_SIZE = 32;
    static const unsigned GENERIC_ALLOC_SIZE = 32;
    // 16, 32, 64, 128, 256 till variable
    // ** Update Allocator.cc if this changes
    static const unsigned NUM_FIXED_ALLOCATORS = 5;

    static constexpr char info_name[] = "graph.jdb";

    struct RegionInfo {
        static const int REGION_NAME_LEN = 32;
        char name[REGION_NAME_LEN];     ///< Region name
        uint64_t addr;                  ///< Virtual address of region
        size_t len;                     ///< Length in byte
    };

#define REGION_ADDRESS(index) (BASE_ADDRESS + (index) * REGION_SIZE)
    static constexpr RegionInfo default_regions[] = {
        { "nodes.jdb", REGION_ADDRESS(1), REGION_SIZE },
        { "edges.jdb", REGION_ADDRESS(2), REGION_SIZE },
        { "pooh-bah.jdb", REGION_ADDRESS(3), NUM_FIXED_ALLOCATORS * REGION_SIZE },
    };

    static constexpr AllocatorInfo default_allocators[] = {
        { 0, REGION_SIZE, 16 },
        { 1*REGION_SIZE, REGION_SIZE, 32 },
        { 2*REGION_SIZE, REGION_SIZE, 64 },
        { 3*REGION_SIZE, REGION_SIZE, 128 },
        { 4*REGION_SIZE, REGION_SIZE, 256 },
    };
#undef REGION_ADDRESS

    struct GraphInfo {
        uint64_t version;

        // node_table, edge_table, property_chunks
        RegionInfo node_info;
        RegionInfo edge_info;
        RegionInfo allocator_info;

        // Transaction info
        // Lock table info
        uint32_t num_fixed_allocators;
        AllocatorInfo fixed_allocator_info[];
    };

    class GraphInit {
        bool _create;
        os::MapRegion _info_map;
        GraphInfo *_info;

    public:
        GraphInit(const char *name, int options);
        bool create() const { return _create; }
        const GraphInfo *info() const { return _info; }
        const RegionInfo &node_info() const { return _info->node_info; }
        const RegionInfo &edge_info() const { return _info->edge_info; }
        const RegionInfo &allocator_info() const { return _info->allocator_info; }
    };

    class MapRegion : public os::MapRegion {
    public:
        MapRegion(const char *db_name, const RegionInfo &info, bool create)
            : os::MapRegion(db_name, info.name, info.addr, info.len, create, create)
        {}
    };

    // ** Order here is important: GraphInit MUST be first
    GraphInit _init;

    // File-backed space
    MapRegion _node_region;
    MapRegion _edge_region;
    MapRegion _allocator_region;

    NodeTable _node_table;
    EdgeTable _edge_table;
    // Other Fixed ones
    // Variable allocator
    Allocator _allocator;

    // Transactions
    // Lock manager

    AllocatorInfo allocator_info(const RegionInfo &info, uint32_t obj_size) const
    {
        // This returns with offset in region 0. The ones that 
        // need specific offset are anyway stored explicitly in the
        // GraphInfo struct
        AllocatorInfo r = {0, info.len, obj_size, false};
        return r;
    }
public:
    GraphImpl(const char *name, int options)
        : _init(name, options),
          _node_region(name, _init.info()->node_info, _init.create()),
          _edge_region(name, _init.info()->edge_info, _init.create()),
          _allocator_region(name, _init.info()->allocator_info, _init.create()),
          _node_table(_init.node_info().addr,
                      allocator_info(_init.node_info(), NODE_SIZE),
                      _init.create()),
          _edge_table(_init.edge_info().addr,
                      allocator_info(_init.edge_info(), EDGE_SIZE),
                      _init.create()),
          _allocator(_init.allocator_info().addr,
                     _init.info()->fixed_allocator_info,
                     _init.info()->num_fixed_allocators,
                     _init.create())
        { }
    NodeTable &node_table() { return _node_table; }
    EdgeTable &edge_table() { return _edge_table; }
    Allocator &allocator() { return _allocator; }
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
    node->init(tag, _impl->node_table().object_size(), _impl->allocator());
    return *node;
}

Jarvis::Edge &Jarvis::Graph::add_edge(Node &src, Node &dest, StringID tag)
{
    Edge *edge = (Edge *)_impl->edge_table().alloc();
    edge->init(src, dest, tag, _impl->edge_table().object_size());
    src.add_edge(edge, OUTGOING, tag, _impl->allocator());
    dest.add_edge(edge, INCOMING, tag, _impl->allocator());
    return *edge;
}


constexpr char Jarvis::Graph::GraphImpl::info_name[];
constexpr Jarvis::Graph::GraphImpl::RegionInfo Jarvis::Graph::GraphImpl::default_regions[];
constexpr Jarvis::AllocatorInfo Jarvis::Graph::GraphImpl::default_allocators[];

Jarvis::Graph::GraphImpl::GraphInit::GraphInit(const char *name, int options)
    : _create(options & Create),
      _info_map(name, info_name, BASE_ADDRESS, INFO_SIZE, _create, false),
      _info(reinterpret_cast<GraphInfo *>(BASE_ADDRESS))
{
    // _create was modified by _info_map constructor
    // depending on whether the file existed or not

    // Set up the info structure
    if (_create) {
        // Version info
        _info->version = 1;

        // TODO replace static indexing
        _info->node_info = default_regions[0];
        _info->edge_info = default_regions[1];
        // Other information
        _info->allocator_info = default_regions[2];
        _info->num_fixed_allocators = NUM_FIXED_ALLOCATORS;
        memcpy(_info->fixed_allocator_info, default_allocators, sizeof default_allocators);
    }
}

namespace Jarvis {
    template <typename B, typename T>
    class Graph_Iterator : public B {
        const FixedAllocator &table;
        void _next();
        void _skip();

    protected:
        T *_cur;

    public:
        Graph_Iterator(const FixedAllocator &);
        operator bool() const { return _cur != NULL; }
        bool next();
    };

    class Graph_NodeIterator : public Graph_Iterator<NodeIteratorImpl, Node> {
    public:
        Graph_NodeIterator(const FixedAllocator &a)
            : Graph_Iterator<NodeIteratorImpl, Node>(a)
            { }
        const Node &operator*() const { return *_cur; }
        const Node *operator->() const { return _cur; }
        Node &operator*() { return *_cur; }
        Node *operator->() { return _cur; }
    };

    class Graph_EdgeIterator : public Graph_Iterator<EdgeIteratorImpl, Edge> {
        EdgeRef _ref;

        friend class EdgeRef;
        Edge *get_edge() const { return (Edge *)_cur; }
        StringID get_tag() const { return get_edge()->get_tag(); }
        Node &get_source() const { return get_edge()->get_source(); }
        Node &get_destination() const { return get_edge()->get_destination(); }
    public:
        Graph_EdgeIterator(const FixedAllocator &a)
            : Graph_Iterator<EdgeIteratorImpl, Edge>(a), _ref(this)
            {}
        const EdgeRef &operator*() const { return _ref; }
        const EdgeRef *operator->() const { return &_ref; }
        EdgeRef &operator*() { return _ref; }
        EdgeRef *operator->() { return &_ref; }
    };
};

template <typename B, typename T>
Jarvis::Graph_Iterator<B, T>::Graph_Iterator(const FixedAllocator &n)
    : table(n)
{
    _cur = static_cast<T *>(table.begin());
    _next();
}

template <typename B, typename T>
bool Jarvis::Graph_Iterator<B, T>::next()
{
    _skip();
    _next();
    return _cur != NULL;
}

template <typename B, typename T>
void Jarvis::Graph_Iterator<B, T>::_next()
{
    while (_cur < table.end() && table.is_free(_cur))
        _skip();

    if (_cur >= table.end())
        _cur = NULL;
}

template <typename B, typename T>
void Jarvis::Graph_Iterator<B, T>::_skip()
{
    _cur = static_cast<T *>(table.next(_cur));
}

Jarvis::NodeIterator Jarvis::Graph::get_nodes()
{
    return NodeIterator(new Graph_NodeIterator(_impl->node_table()));
}

Jarvis::EdgeIterator Jarvis::Graph::get_edges()
{
    return EdgeIterator(new Graph_EdgeIterator(_impl->edge_table()));
}

Jarvis::NodeID Jarvis::Graph::get_id(const Node &node) const
{
    return _impl->node_table().get_id(&node);
}

Jarvis::EdgeID Jarvis::Graph::get_id(const Edge &edge) const
{
    return _impl->edge_table().get_id(&edge);
}
