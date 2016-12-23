#pragma once
#include "stringid.h"
#include "ChunkList.h"
#include "property.h"
#include "Index.h"
#include "node.h"
#include "edge.h"
#include "iterator.h"
#include "callback.h"

namespace Jarvis {
    class Allocator;

    // This class creates/maintains all indexes in Jarvis.
    // It supports the create_index() API visible to the user
    // There are separate indices for nodes and edges.
    // Each node/edge index first goes through its tag (default = 0
    // when no tag specified) and then looks for a property id based
    // index data structure.
    // The final index structure indexes based on property values
    class IndexManager {
        static const unsigned TAGLIST_CHUNK_SIZE = 128;
        static const unsigned INDEXLIST_CHUNK_SIZE = 128;

        // A pair for the property id being indexed and its corresponding
        // value tree.
        // Use a base Index class here since the template types for the
        // actual ADTs are known only at the time of creation.
        // A chunklist for storing the above K,V pair
        typedef ChunkList<StringID, Index *, INDEXLIST_CHUNK_SIZE> IndexList;
        typedef ChunkList<StringID, IndexList, TAGLIST_CHUNK_SIZE> TagList;

        // A pair for the node or edge tag that will have multiple property
        // indexes
        // An array of 2 chunk lists, NodeIndex first, EdgeIndex next
        // This is a pointer so we can typecast it in PM at the constructor
        TagList *_tag_prop_map;

        CallbackList<void *, void *> _iterator_remove_list;
        CallbackList<void *, void *> _iterator_rebalance_list;
        CallbackList<void *, const PropertyRef &> _property_iterator_list;

        IndexList *add_tag_index(Graph::IndexType index_type,
                                     StringID tag,
                                     Allocator &allocator);

        bool add(Graph::IndexType index_type, StringID tag, void *obj,
                 Allocator &allocator);
        void remove(Graph::IndexType index_type, StringID tag, void *obj,
                 Allocator &allocator);

        Graph::IndexStats get_index_stats(IndexList *tag_entry);

    public:
        IndexManager(const uint64_t region_addr, bool create)
            : _tag_prop_map(reinterpret_cast<TagList *>(region_addr))
        {
            if (create) {
                _tag_prop_map[0].init();
                _tag_prop_map[1].init();
            }
        }

        void create_index(Graph::IndexType index_type, StringID tag,
                            StringID property_id,
                            PropertyType ptype,
                            Allocator &allocator);

        // Nodes and edges have to be added to an index in two
        // stages. One at the add_node or add_edge stage. Another at
        // the set_property state. We cannot wait until first set_property
        // since the user might just want to query based on tag and
        // not set any property until then.
        bool add_node(Node *node, Allocator &allocator)
            { return add(Graph::NodeIndex, node->get_tag(), node, allocator); }

        bool add_edge(Edge *edge, Allocator &allocator)
            { return add(Graph::EdgeIndex, edge->get_tag(), edge, allocator); }

        void remove_node(Node *node, Allocator &allocator)
            { remove(Graph::NodeIndex, node->get_tag(), node, allocator); }

        void remove_edge(Edge *edge, Allocator &allocator)
            { remove(Graph::EdgeIndex, edge->get_tag(), edge, allocator); }

        void update(GraphImpl *db,
                    Graph::IndexType index_type, StringID tag, void *obj,
                    StringID id,
                    const PropertyRef *old_value, const Property *new_value);

        Index *get_index(Graph::IndexType index_type, StringID tag,
                         StringID property_id,
                         PropertyType ptype = PropertyType(0));

        Graph::IndexStats get_index_stats();
        Graph::IndexStats get_index_stats(Graph::IndexType index_type);
        Graph::IndexStats get_index_stats(Graph::IndexType index_type, StringID tag);
        Graph::IndexStats get_index_stats(Graph::IndexType index_type, StringID tag,
                               StringID property_id);

        Graph::ChunkStats get_all_chunk_lists_stats();
        Graph::ChunkStats get_chunk_list_stats(Graph::IndexType index_type);
        Graph::ChunkStats get_chunk_list_stats(Graph::IndexType index_type, StringID tag);

        Index::Index_IteratorImplIntf *get_iterator(Graph::IndexType index_type,
                                                    StringID tag);

        void register_iterator(void *key,
                               std::function<void(void *)> remove_callback)
        {
            _iterator_remove_list.register_callback(key, remove_callback);
        }

        void register_iterator(void *key,
                               std::function<void(void *)> remove_callback,
                               std::function<void(void *)> rebalance_callback)
        {
            _iterator_remove_list.register_callback(key, remove_callback);
            _iterator_rebalance_list.register_callback(key, rebalance_callback);
        }

        void unregister_iterator(void *key)
        {
            _iterator_remove_list.unregister_callback(key);
            _iterator_rebalance_list.unregister_callback(key);
        }

        void iterator_remove_notify(void *list_node) const
            { _iterator_remove_list.do_callbacks(list_node); }
        void iterator_rebalance_notify(void *tree) const
            { _iterator_rebalance_list.do_callbacks(tree); }

        void register_property_iterator(void *key, std::function<void(const PropertyRef &)> f)
            { _property_iterator_list.register_callback(key, f); }
        void unregister_property_iterator(void *key)
            { _property_iterator_list.unregister_callback(key); }
        void property_iterator_notify(const PropertyRef &p) const
            { _property_iterator_list.do_callbacks(p); }
    };
}
