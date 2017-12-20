/**
 * @file   graph.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stddef.h>
#include <string.h>
#include "graph.h"
#include "GraphConfig.h"
#include "GraphImpl.h"
#include "node.h"
#include "edge.h"
#include "Allocator.h"
#include "iterator.h"
#include "TransactionImpl.h"
#include "TransactionManager.h"
#include "arch.h"
#include "os.h"
#include "Index.h"
#include "filter.h"

using namespace PMGD;

static constexpr char info_name[] = "graph.jdb";

extern constexpr char commit_id[] = "Commit id: " COMMIT_ID;

struct GraphImpl::GraphInfo {
    static const uint64_t VERSION = 8;

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

    // We store allocator region information in the graph header
    // to avoid using pages within the allocator pools and avoid
    // wasting space due to alignment constraints.
    Allocator::RegionHeader allocator_hdr;

    void init(const GraphConfig &, bool, RangeSet &);

    GraphInfo(const GraphInfo &) = delete;
    ~GraphInfo() = delete;
    void operator=(const GraphInfo &) = delete;
};

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
    // The FixedAllocator doesn't implement its own locking. So lock
    // it here such that it is reusable by the same transaction in case
    // it needs to allocate multiple nodes.
    TransactionImpl *tx = TransactionImpl::get_tx();
    GraphImpl::NodeTable &ntable = _impl->node_table();
    tx->acquire_lock(TransactionImpl::NodeLock, &ntable, true);
    Node *node = (Node *)ntable.alloc();
    node->init(tag, ntable.object_size(), _impl->allocator());
    _impl->index_manager().add_node(node, _impl->allocator());
    return *node;
}

Edge &Graph::add_edge(Node &src, Node &dest, StringID tag)
{
    TransactionImpl *tx = TransactionImpl::get_tx();
    GraphImpl::EdgeTable &etable = _impl->edge_table();
    tx->acquire_lock(TransactionImpl::EdgeLock, &etable, true);
    Edge *edge = (Edge *)etable.alloc();
    edge->init(src, dest, tag, etable.object_size());
    src.add_edge(edge, Outgoing, tag, _impl->allocator());
    dest.add_edge(edge, Incoming, tag, _impl->allocator());
    _impl->index_manager().add_edge(edge, _impl->allocator());
    return *edge;
}

void Graph::remove(Node &node)
{
    TransactionImpl *tx = TransactionImpl::get_tx();
    GraphImpl::NodeTable &ntable = _impl->node_table();
    tx->acquire_lock(TransactionImpl::NodeLock, &ntable, true);
    tx->acquire_lock(TransactionImpl::NodeLock, &node, true);

    // Remove edges before properties to ensure we can get all locks
    // before doing the work.
    node.get_edges().process([this](Edge &edge) { remove(edge); });

    Allocator &allocator = _impl->allocator();
    _impl->index_manager().remove_node(&node, allocator);

    node.remove_all_properties();
    node.cleanup(allocator);
    ntable.free(&node);
}

void Graph::remove(Edge &edge)
{
    TransactionImpl *tx = TransactionImpl::get_tx();
    GraphImpl::EdgeTable &etable = _impl->edge_table();
    tx->acquire_lock(TransactionImpl::EdgeLock, &etable, true);
    tx->acquire_lock(TransactionImpl::EdgeLock, &edge, true);
    Allocator &allocator = _impl->allocator();
    edge.get_source().remove_edge(&edge, Outgoing, allocator);
    edge.get_destination().remove_edge(&edge, Incoming, allocator);
    _impl->index_manager().remove_edge(&edge, allocator);

    // Remove edge from nodes before properties to ensure we can get all locks
    // before doing the work.
    edge.remove_all_properties();
    etable.free(&edge);
}

void Graph::create_index(IndexType index_type, StringID tag,
                         StringID property_id, const PropertyType ptype)
{
    _impl->index_manager().create_index(index_type, tag,
                                        property_id, ptype, _impl->allocator());
}

GraphImpl::GraphInit::GraphInit(const char *name, int options,
                                const Graph::Config *user_config)
    : params{(options & Graph::Create), (options & Graph::ReadOnly),
              false, false, new RangeSet()},
      info_map(name, info_name,
               GraphConfig::BASE_ADDRESS, GraphConfig::INFO_SIZE,
               params.create, false, params.read_only),
      info(reinterpret_cast<GraphInfo *>(GraphConfig::BASE_ADDRESS))
{
    int msync_options = options & Graph::AlwaysMsync;
    switch (msync_options) {
    case 0:
    case Graph::MsyncOnCommit:
        params.msync_needed = true;
        params.always_msync = false;
        break;
    case Graph::AlwaysMsync:
        params.msync_needed = params.always_msync = true;
        break;
    case Graph::NoMsync:
        params.msync_needed = params.always_msync = false;
    }

    // Since the lock variables are also calculated here and they
    // have to be set regardless of create, initialize this struct
    // outside of the if.
    const GraphConfig config(user_config);

    // create was modified by _info_map constructor
    // depending on whether the file existed or not
    // For a new graph, initialize the info structure
    if (params.create) {
        info->init(config, params.msync_needed, *params.pending_commits);
        node_size = config.node_size;
        edge_size = config.edge_size;
        num_allocators = config.num_allocators;
    }
    else {
        if (info->version != GraphInfo::VERSION)
            throw PMGDException(VersionMismatch);
    }

    node_striped_lock_size = config.node_striped_lock_size;
    edge_striped_lock_size = config.edge_striped_lock_size;
    index_striped_lock_size = config.index_striped_lock_size;
    node_stripe_width = config.node_stripe_width;
    edge_stripe_width = config.edge_stripe_width;
    index_stripe_width = config.index_stripe_width;
}

void GraphImpl::GraphInfo::init(const GraphConfig &config,
                                bool msync_needed,
                                RangeSet &pending_commits)
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
        throw PMGDException(InvalidConfig);
    memcpy(locale_name, config.locale_name.c_str(), size);

    TransactionImpl::flush_range(this, sizeof *this, msync_needed, pending_commits);
}

GraphImpl::MapRegion::MapRegion(
        const char *db_name, const RegionInfo &info, bool create, bool read_only)
    : os::MapRegion(db_name, info.name, info.addr, info.len, create, create, read_only)
{
}

GraphImpl::GraphImpl(const char *name, int options, const Graph::Config *config)
    : _init(name, options, config),
      _transaction_region(name, _init.info->transaction_info,
                          _init.params.create, _init.params.read_only),
      _journal_region(name, _init.info->journal_info,
                      _init.params.create, _init.params.read_only),
      _indexmanager_region(name, _init.info->indexmanager_info,
                           _init.params.create, _init.params.read_only),
      _stringtable_region(name, _init.info->stringtable_info,
                          _init.params.create, _init.params.read_only),
      _node_region(name, _init.info->node_info, _init.params.create, _init.params.read_only),
      _edge_region(name, _init.info->edge_info, _init.params.create, _init.params.read_only),
      _allocator_region(name, _init.info->allocator_info,
                        _init.params.create, _init.params.read_only),
      _transaction_manager(_init.info->transaction_info.addr,
                           _init.info->transaction_info.len,
                           _init.info->journal_info.addr,
                           _init.info->journal_info.len,
                           _init.params),
      _index_manager(_init.info->indexmanager_info.addr, _init.params),
      _string_table(_init.info->stringtable_info.addr,
                    _init.info->stringtable_info.len,
                    _init.info->max_stringid_length,
                    _init.params),
      _node_table(_init.info->node_info.addr,
                  _init.node_size, _init.info->node_info.len,
                  _init.params),
      _edge_table(_init.info->edge_info.addr,
                  _init.edge_size, _init.info->edge_info.len,
                  _init.params),
      _allocator(this, _init.info->allocator_info.addr,
                 _init.info->allocator_info.len,
                 &_init.info->allocator_hdr,
                 _init.num_allocators,
                 _init.params),
      _locale(_init.info->locale_name[0] != '\0'
                  ? std::locale(_init.info->locale_name)
                  : std::locale()),
      _node_locks(_init.node_striped_lock_size, _init.node_stripe_width),
      _edge_locks(_init.edge_striped_lock_size, _init.edge_stripe_width),
      _index_locks(_init.index_striped_lock_size, _init.index_stripe_width)
{
    TransactionManager::commit(_init.params.msync_needed, *_init.params.pending_commits);
}

namespace PMGD {
    template <typename B, typename T>
    class Graph_Iterator : public B {
        const FixedAllocator &table;
        void _next();
        void _skip();

    protected:
        T *_cur;
        void check_vacant();

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

        Node *ref()
        {
            check_vacant();
            TransactionImpl::lock_node(_cur, false);
            return _cur;
        }
    };

    class Graph_EdgeIteratorImpl : public Graph_Iterator<EdgeIteratorImplIntf, Edge> {
        EdgeRef _ref;

        friend class EdgeRef;
        Edge *get_edge() const
        {
            TransactionImpl::lock_edge(this, false);
            return (Edge *)_cur;
        }

        StringID get_tag() const { return get_edge()->get_tag(); }
        Node &get_source() const { return get_edge()->get_source(); }
        Node &get_destination() const { return get_edge()->get_destination(); }
    public:
        Graph_EdgeIteratorImpl(const FixedAllocator &a)
            : Graph_Iterator<EdgeIteratorImplIntf, Edge>(a), _ref(this)
            {}

        EdgeRef *ref()
        {
            check_vacant();
            return &_ref;
        }
    };

    class Index_NodeIteratorImpl : public NodeIteratorImplIntf {
        Index::Index_IteratorImplIntf *_iter;
    public:
        Index_NodeIteratorImpl(Index::Index_IteratorImplIntf *iter)
            : _iter(iter) { }
        ~Index_NodeIteratorImpl() { delete _iter; }
        Node *ref() { return (Node *)_iter->ref(); }
        operator bool() const { return _iter && *_iter; }
        bool next() { return _iter->next(); }
    };

    class Index_EdgeIteratorImpl : public EdgeIteratorImplIntf {
        Index::Index_IteratorImplIntf *_iter;
        EdgeRef _ref;

        friend class EdgeRef;
        Edge *get_edge() const { return (Edge *)_iter->ref(); }
        StringID get_tag() const { return get_edge()->get_tag(); }
        Node &get_source() const { return get_edge()->get_source(); }
        Node &get_destination() const { return get_edge()->get_destination(); }
    public:
        Index_EdgeIteratorImpl(Index::Index_IteratorImplIntf *iter)
            : _iter(iter), _ref(this) { }
        ~Index_EdgeIteratorImpl() { delete _iter; }
        EdgeRef *ref() { return &_ref; }
        operator bool() const { return _iter && *_iter; }
        bool next() { return _iter->next(); }
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

template <typename B, typename T>
void Graph_Iterator<B, T>::check_vacant()
{
    if (table.is_free(_cur))
        throw PMGDException(VacantIterator);
}


NodeIterator Graph::get_nodes()
{
    TransactionImpl *tx = TransactionImpl::get_tx();
    GraphImpl::NodeTable &ntable = _impl->node_table();
    tx->acquire_lock(TransactionImpl::NodeLock, &ntable, false);
    return NodeIterator(new Graph_NodeIteratorImpl(ntable));
}

NodeIterator Graph::get_nodes(StringID tag)
{
    if (tag.id() == 0)
        return get_nodes();
    else
        return NodeIterator(new Index_NodeIteratorImpl(_impl->index_manager().get_iterator(NodeIndex, tag)));
}

NodeIterator Graph::get_nodes(StringID tag, const PropertyPredicate &pp, bool reverse)
{
    if (pp.id == 0)
        return get_nodes(tag);
    Index *index = _impl->index_manager().get_index(NodeIndex, tag, pp.id);
    if (index)
        return NodeIterator(new Index_NodeIteratorImpl(index->get_iterator(NodeIndex, pp, &_impl->locale(), reverse)));
    else
        return get_nodes(tag).filter(pp); // TODO Causes re-lookup of tag
}


EdgeIterator Graph::get_edges()
{
    TransactionImpl *tx = TransactionImpl::get_tx();
    GraphImpl::EdgeTable &etable = _impl->edge_table();
    tx->acquire_lock(TransactionImpl::EdgeLock, &etable, false);
    return EdgeIterator(new Graph_EdgeIteratorImpl(etable));
}

EdgeIterator Graph::get_edges(StringID tag)
{
    if (tag.id() == 0)
        return get_edges();
    else
        return EdgeIterator(new Index_EdgeIteratorImpl(_impl->index_manager().get_iterator(EdgeIndex, tag)));
}

EdgeIterator Graph::get_edges(StringID tag, const PropertyPredicate &pp, bool reverse)
{
    if (pp.id == 0)
        return get_edges(tag);
    Index *index = _impl->index_manager().get_index(EdgeIndex, tag, pp.id);
    if (index)
        return EdgeIterator(new Index_EdgeIteratorImpl(index->get_iterator(EdgeIndex, pp, &_impl->locale(), reverse)));
    else
        return get_edges(tag).filter(pp); // TODO Causes re-lookup of tag
}

NodeID Graph::get_id(const Node &node) const
{
    // The node id acquired from node_table which will not change unless
    // the node is being removed. So read lock the node.
    TransactionImpl::lock_node(&node, false);
    return _impl->node_table().get_id(&node);
}

EdgeID Graph::get_id(const Edge &edge) const
{
    // The edge id acquired from the edge table which will not change unless
    // the edge is being removed. So read lock the edge.
    TransactionImpl::lock_edge(&edge, false);
    return _impl->edge_table().get_id(&edge);
}

// Stats Interface
Graph::IndexStats Graph::get_index_stats()
{
    return _impl->index_manager().get_index_stats();
}

Graph::IndexStats Graph::get_index_stats(Graph::IndexType index_type)
{
    return _impl->index_manager().get_index_stats(index_type);
}

Graph::IndexStats Graph::get_index_stats(Graph::IndexType index_type, StringID tag)
{
    return _impl->index_manager().get_index_stats(index_type, tag);
}

Graph::IndexStats Graph::get_index_stats(Graph::IndexType index_type,
                                   StringID tag, StringID property_id)
{
    return _impl->index_manager().get_index_stats(index_type, tag, property_id);
}

Graph::ChunkStats Graph::get_all_chunk_lists_stats()
{
    return _impl->index_manager().get_all_chunk_lists_stats();
}

Graph::ChunkStats Graph::get_chunk_list_stats(Graph::IndexType index_type)
{
    return _impl->index_manager().get_chunk_list_stats(index_type);
}

Graph::ChunkStats Graph::get_chunk_list_stats(Graph::IndexType index_type, StringID tag)
{
    return _impl->index_manager().get_chunk_list_stats(index_type, tag);
}

std::vector<Graph::AllocatorStats> Graph::get_allocator_stats()
{
    std::vector<AllocatorStats> stats;

    stats.push_back(AllocatorStats{ "NodeTable",
                                    _impl->node_table().object_size(),
                                    (unsigned long long) _impl->node_table().num_allocated(),
                                    _impl->node_table().used_bytes(),
                                    _impl->node_table().region_size(),
                                    _impl->node_table().occupancy(),
                                    _impl->node_table().health() });

    stats.push_back(AllocatorStats{ "EdgeTable",
                                    _impl->edge_table().object_size(),
                                    (unsigned long long) _impl->edge_table().num_allocated(),
                                    _impl->edge_table().used_bytes(),
                                    _impl->edge_table().region_size(),
                                    _impl->edge_table().occupancy(),
                                    _impl->edge_table().health() });

    stats.push_back(AllocatorStats{ "GenericAllocator",
                                    0,
                                    0,
                                    _impl->allocator().used_bytes(),
                                    _impl->allocator().region_size(),
                                    _impl->allocator().occupancy(),
                                    _impl->allocator().health() });

    return stats;
}
