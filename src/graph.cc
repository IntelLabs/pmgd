#include <stddef.h>
#include <string.h>
#include "graph.h"
#include "GraphConfig.h"
#include "GraphImpl.h"
#include "node.h"
#include "edge.h"
#include "allocator.h"
#include "iterator.h"
#include "TransactionImpl.h"
#include "TransactionManager.h"
#include "arch.h"
#include "os.h"
#include "Index.h"
#include "filter.h"

using namespace Jarvis;

static constexpr char info_name[] = "graph.jdb";

struct GraphImpl::GraphInfo {
    static const uint64_t VERSION = 2;

    uint64_t version;

    RegionInfo transaction_info;
    RegionInfo journal_info;
    RegionInfo indexmanager_info;
    RegionInfo stringtable_info;
    RegionInfo node_info;
    RegionInfo edge_info;
    RegionInfo allocator_info;

    uint32_t max_stringid_length;

    char locale_name[32];

    uint32_t num_fixed_allocators;
    uint64_t allocator_offsets[];

    void init(const GraphConfig &);
};

namespace Jarvis {
const unsigned GraphImpl::MAX_FIXED_ALLOCATORS
    = (GraphConfig::INFO_SIZE - offsetof(GraphInfo, allocator_offsets))
               / sizeof GraphInfo::allocator_offsets[0]
           - 1;
}


Graph::Graph(const char *name, int options, const Config *config)
    : _impl(new GraphImpl(name, options, config))
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
    _impl->index_manager().add_node(node, _impl->allocator());
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

void Graph::create_index(int node_or_edge, StringID tag,
                         StringID property_id, const PropertyType ptype)
{
    _impl->index_manager().create_index(node_or_edge, tag,
                                        property_id, ptype, _impl->allocator());
}

GraphImpl::GraphInit::GraphInit(const char *name, int options,
                                const Graph::Config *user_config)
    : create(options & Graph::Create),
      read_only(options & Graph::ReadOnly),
      info_map(name, info_name,
               GraphConfig::BASE_ADDRESS, GraphConfig::INFO_SIZE,
               create, false, read_only),
      info(reinterpret_cast<GraphInfo *>(GraphConfig::BASE_ADDRESS))
{
    // create was modified by _info_map constructor
    // depending on whether the file existed or not
    // For a new graph, initialize the info structure
    if (create) {
        const GraphConfig config(user_config);
        info->init(config);
        node_size = config.node_size;
        edge_size = config.edge_size;
        fixed_allocator_info = std::move(config.fixed_allocator_info);
    }
    else {
        if (info->version != GraphInfo::VERSION)
            throw Exception(version_mismatch);
    }
}

void GraphImpl::GraphInfo::init(const GraphConfig &config)
{
    version = VERSION;
    transaction_info = config.transaction_info;
    journal_info = config.journal_info;
    indexmanager_info = config.indexmanager_info;
    stringtable_info = config.stringtable_info;
    node_info = config.node_info;
    edge_info = config.edge_info;
    allocator_info = config.allocator_info;

    max_stringid_length = config.max_stringid_length;

    unsigned size = config.locale_name.length() + 1;
    if (size > sizeof locale_name)
        throw Exception(not_implemented);
    memcpy(locale_name, config.locale_name.c_str(), size);

    num_fixed_allocators = config.fixed_allocator_info.size();
    for (unsigned i = 0; i < num_fixed_allocators; i++)
        allocator_offsets[i] = config.fixed_allocator_info[i].offset;

    TransactionImpl::flush_range(this, sizeof *this);
}

GraphImpl::MapRegion::MapRegion(
        const char *db_name, const RegionInfo &info, bool create, bool read_only)
    : os::MapRegion(db_name, info.name, info.addr, info.len, create, create, read_only)
{
}

GraphImpl::GraphImpl(const char *name, int options, const Graph::Config *config)
    : _init(name, options, config),
      _transaction_region(name, _init.info->transaction_info, _init.create, _init.read_only),
      _journal_region(name, _init.info->journal_info, _init.create, _init.read_only),
      _indexmanager_region(name, _init.info->indexmanager_info, _init.create, _init.read_only),
      _stringtable_region(name, _init.info->stringtable_info, _init.create, _init.read_only),
      _node_region(name, _init.info->node_info, _init.create, _init.read_only),
      _edge_region(name, _init.info->edge_info, _init.create, _init.read_only),
      _allocator_region(name, _init.info->allocator_info, _init.create, _init.read_only),
      _transaction_manager(_init.info->transaction_info.addr,
                           _init.info->transaction_info.len,
                           _init.info->journal_info.addr,
                           _init.info->journal_info.len,
                           _init.create, _init.read_only),
      _index_manager(_init.info->indexmanager_info.addr, _init.create),
      _string_table(_init.info->stringtable_info.addr,
                    _init.info->stringtable_info.len,
                    _init.info->max_stringid_length,
                    _init.create),
      _node_table(_init.info->node_info.addr,
                  _init.node_size, _init.info->node_info.len,
                  _init.create),
      _edge_table(_init.info->edge_info.addr,
                  _init.edge_size, _init.info->edge_info.len,
                  _init.create),
      _allocator(_init.info->allocator_info.addr,
                 _init.info->num_fixed_allocators,
                 _init.info->allocator_offsets,
                 &_init.fixed_allocator_info[0],
                 _init.create),
      _locale(_init.info->locale_name[0] != '\0'
                  ? std::locale(_init.info->locale_name)
                  : std::locale())
{
    persistent_barrier(11);
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

    class Graph_NodeIteratorImpl : public Graph_Iterator<NodeIteratorImplIntf, Node> {
    public:
        Graph_NodeIteratorImpl(const FixedAllocator &a)
            : Graph_Iterator<NodeIteratorImplIntf, Node>(a)
            { }
        const Node &operator*() const { return *_cur; }
        const Node *operator->() const { return _cur; }
        Node &operator*() { return *_cur; }
        Node *operator->() { return _cur; }
    };

    class Graph_EdgeIteratorImpl : public Graph_Iterator<EdgeIteratorImplIntf, Edge> {
        EdgeRef _ref;

        friend class EdgeRef;
        Edge *get_edge() const { return (Edge *)_cur; }
        StringID get_tag() const { return get_edge()->get_tag(); }
        Node &get_source() const { return get_edge()->get_source(); }
        Node &get_destination() const { return get_edge()->get_destination(); }
    public:
        Graph_EdgeIteratorImpl(const FixedAllocator &a)
            : Graph_Iterator<EdgeIteratorImplIntf, Edge>(a), _ref(this)
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
    return NodeIterator(new Graph_NodeIteratorImpl(_impl->node_table()));
}

NodeIterator Graph::get_nodes(StringID tag)
{
    if (tag.id() == 0)
        return get_nodes();
    else
        return _impl->index_manager().get_nodes(tag);
}

NodeIterator Graph::get_nodes(StringID tag, const PropertyPredicate &pp)
{
    Index *index = _impl->index_manager().get_index(NODE, tag, pp.id);
    if (index)
        return index->get_nodes(pp, &_impl->locale());
    else
        return get_nodes(tag).filter(pp); // TODO Causes re-lookup of tag
}

EdgeIterator Graph::get_edges()
{
    return EdgeIterator(new Graph_EdgeIteratorImpl(_impl->edge_table()));
}

NodeID Graph::get_id(const Node &node) const
{
    return _impl->node_table().get_id(&node);
}

EdgeID Graph::get_id(const Edge &edge) const
{
    return _impl->edge_table().get_id(&edge);
}
