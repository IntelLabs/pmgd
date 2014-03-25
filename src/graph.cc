#include <stddef.h>
#include <string.h>
#include "graph.h"
#include "GraphImpl.h"
#include "node.h"
#include "edge.h"
#include "allocator.h"
#include "iterator.h"
#include "TransactionImpl.h"
#include "TransactionManager.h"
#include "arch.h"
#include "os.h"

using namespace Jarvis;

static const size_t BASE_ADDRESS = 0x10000000000;
static const size_t REGION_SIZE = 0x10000000000;
static const unsigned NODE_SIZE = 64;
static const unsigned EDGE_SIZE = 32;

static constexpr char info_name[] = "graph.jdb";

struct GraphImpl::RegionInfo {
    static const int REGION_NAME_LEN = 32;
    char name[REGION_NAME_LEN];     ///< Region name
    uint64_t addr;                  ///< Virtual address of region
    size_t len;                     ///< Length in byte
};

struct GraphImpl::GraphInfo {
    uint64_t version;

    RegionInfo transaction_info;
    RegionInfo node_info;
    RegionInfo edge_info;
    RegionInfo allocator_info;
    RegionInfo stringtable_info;

    uint32_t num_fixed_allocators;
    AllocatorInfo fixed_allocator_info[];
};

const AllocatorInfo GraphImpl::default_allocators[] = {
    { 0, REGION_SIZE, 16 },
    { 1*REGION_SIZE, REGION_SIZE, 32 },
    { 2*REGION_SIZE, REGION_SIZE, 64 },
    { 3*REGION_SIZE, REGION_SIZE, 128 },
    { 4*REGION_SIZE, REGION_SIZE, 256 },
};
const size_t GraphImpl::NUM_FIXED_ALLOCATORS
        = sizeof default_allocators / sizeof default_allocators[0];

#define ADDRESS(region) (region##_ADDRESS)
#define SIZE(region) (region##_SIZE)
#define NEXT(region) (ADDRESS(region) + SIZE(region))

// Set individual addresses so they can be set at different sizes when needed
static const size_t INFO_ADDRESS = BASE_ADDRESS;
static const unsigned INFO_SIZE = 4096;
static const size_t TRANSACTIONTABLE_ADDRESS = NEXT(INFO);
static const unsigned TRANSACTIONTABLE_SIZE = TRANSACTION_REGION_SIZE;
static const size_t STRINGTABLE_ADDRESS = NEXT(TRANSACTIONTABLE);
// Belongs here to make sure info header contains the
// correct "obj_size"
static const int MAX_STRINGID_CHARLEN = 16;
// 12bits had fewest collisions in testing
static const int MAX_STRINGIDS = 4096;
static const size_t STRINGTABLE_SIZE = MAX_STRINGIDS * MAX_STRINGID_CHARLEN;

// Node, edge tables kept TB aligned
static const size_t NODETABLE_ADDRESS = BASE_ADDRESS + REGION_SIZE;
static const size_t NODETABLE_SIZE = REGION_SIZE;
static const size_t EDGETABLE_ADDRESS = NEXT(NODETABLE);
static const size_t EDGETABLE_SIZE = REGION_SIZE;
static const size_t ALLOCATORS_ADDRESS = NEXT(EDGETABLE);
static const size_t ALLOCATORS_SIZE = GraphImpl::NUM_FIXED_ALLOCATORS * REGION_SIZE;

const GraphImpl::RegionInfo GraphImpl::default_regions[] = {
    { "transaction.jdb", ADDRESS(TRANSACTIONTABLE), SIZE(TRANSACTIONTABLE)},
    { "stringtable.jdb", ADDRESS(STRINGTABLE), SIZE(STRINGTABLE)},
    { "nodes.jdb", ADDRESS(NODETABLE), SIZE(NODETABLE) },
    { "edges.jdb", ADDRESS(EDGETABLE), SIZE(EDGETABLE) },
    { "pooh-bah.jdb", ADDRESS(ALLOCATORS), SIZE(ALLOCATORS) },
};

AllocatorInfo GraphImpl::allocator_info(const RegionInfo &info,
                                        uint32_t obj_size) const
{
    // This returns an offset of 0. The AllocatorInfo structs that
    // need a non-zero offset are stored explicitly in the
    // GraphInfo struct.
    AllocatorInfo r = {0, info.len, obj_size, false};
    return r;
}


Graph::Graph(const char *name, int options)
    : _impl(new GraphImpl(name, options))
{
}

Graph::~Graph()
{
    delete _impl;
}

Node &Graph::add_node(StringID tag)
{
    Node *node = (Node *)_impl->node_table().alloc();
    node->init(tag, _impl->node_table().object_size(), _impl->allocator());
    return *node;
}

Edge &Graph::add_edge(Node &src, Node &dest, StringID tag)
{
    Edge *edge = (Edge *)_impl->edge_table().alloc();
    edge->init(src, dest, tag, _impl->edge_table().object_size());
    src.add_edge(edge, OUTGOING, tag, _impl->allocator());
    dest.add_edge(edge, INCOMING, tag, _impl->allocator());
    return *edge;
}


GraphImpl::GraphInit::GraphInit(const char *name, int options)
    : create(options & Graph::Create),
      info_map(name, info_name, BASE_ADDRESS, INFO_SIZE, create, false),
      info(reinterpret_cast<GraphInfo *>(BASE_ADDRESS))
{
    // create was modified by _info_map constructor
    // depending on whether the file existed or not
    if (create) {
        // For a new graph, initialize the info structure
        info->version = 1;

        // TODO replace static indexing
        info->transaction_info = default_regions[0];
        info->stringtable_info = default_regions[1];
        info->node_info = default_regions[2];
        info->edge_info = default_regions[3];
        info->allocator_info = default_regions[4];
        info->num_fixed_allocators = NUM_FIXED_ALLOCATORS;
        memcpy(info->fixed_allocator_info, default_allocators, sizeof default_allocators);
        TransactionImpl::flush_range(info, sizeof *info);
    }
}

GraphImpl::MapRegion::MapRegion(
        const char *db_name, const RegionInfo &info, bool create)
    : os::MapRegion(db_name, info.name, info.addr, info.len, create, create)
{
}

GraphImpl::GraphImpl(const char *name, int options)
    : _init(name, options),
      _transaction_region(name, _init.info->transaction_info, _init.create),
      _stringtable_region(name, _init.info->stringtable_info, _init.create),
      _node_region(name, _init.info->node_info, _init.create),
      _edge_region(name, _init.info->edge_info, _init.create),
      _allocator_region(name, _init.info->allocator_info, _init.create),
      _transaction_manager(_init.info->transaction_info.addr,
                           _init.info->transaction_info.len,
                           _init.create),
      _string_table(_init.info->stringtable_info.addr,
                    _init.info->stringtable_info.len,
                    MAX_STRINGID_CHARLEN,
                    _init.create),
      _node_table(_init.info->node_info.addr,
                  allocator_info(_init.info->node_info, NODE_SIZE),
                  _init.create),
      _edge_table(_init.info->edge_info.addr,
                  allocator_info(_init.info->edge_info, EDGE_SIZE),
                  _init.create),
      _allocator(_init.info->allocator_info.addr,
                 _init.info->fixed_allocator_info,
                 _init.info->num_fixed_allocators,
                 _init.create)
{
    persistent_barrier();
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
Graph_Iterator<B, T>::Graph_Iterator(const FixedAllocator &n)
    : table(n)
{
    _cur = static_cast<T *>(table.begin());
    _next();
}

template <typename B, typename T>
bool Graph_Iterator<B, T>::next()
{
    _skip();
    _next();
    return _cur != NULL;
}

template <typename B, typename T>
void Graph_Iterator<B, T>::_next()
{
    while (_cur < table.end() && table.is_free(_cur))
        _skip();

    if (_cur >= table.end())
        _cur = NULL;
}

template <typename B, typename T>
void Graph_Iterator<B, T>::_skip()
{
    _cur = static_cast<T *>(table.next(_cur));
}

NodeIterator Graph::get_nodes()
{
    return NodeIterator(new Graph_NodeIterator(_impl->node_table()));
}

EdgeIterator Graph::get_edges()
{
    return EdgeIterator(new Graph_EdgeIterator(_impl->edge_table()));
}

NodeID Graph::get_id(const Node &node) const
{
    return _impl->node_table().get_id(&node);
}

EdgeID Graph::get_id(const Edge &edge) const
{
    return _impl->edge_table().get_id(&edge);
}
