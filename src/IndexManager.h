#pragma once
#include "stringid.h"
#include "ChunkList.h"
#include "property.h"
#include "allocator.h"
#include "List.h"
#include "AvlTreeIndex.h"
#include "KeyValuePair.h"
#include "node.h"
#include "edge.h"
#include "Index.h"
#include "iterator.h"

namespace Jarvis {
    class Index;
    template<typename K, typename V, unsigned S> class ChunkList;
    // This class will create/maintain all indexes in Jarvis.
    // It supports the create_index() API visible to the user
    // There are separate indices for nodes and edges.
    // Each node/edge index first goes through its tag (default = 0
    // when no tag specified) and then looks for a property id based
    // index data structure.
    // The final index structure indexes based on property values
    class IndexManager {

        // A pair for the property id being indexed and it's corresponding
        // value tree.
        // Use a base Index class here since the template types for the
        // actual ADTs are known only at the time of creation.
        // A chunklist for storing the above K,V pair
        typedef ChunkList<StringID, Index *, 128> IndexList;
        typedef ChunkList<StringID, IndexList, 128> TagList;

        // A pair for the node or edge tag that will have multiple property
        // indexes
        // An array of 2 chunk lists NODE first, edge next
        // TODO It might make sense to inline the first chunk in the index
        // page mapped from graph.cc to avoid the first pointer chase beyond
        // the same page.
        TagList *_tag_prop_map;

        bool _create;
        bool _initialized;

        IndexList *add_tag_index(int node_or_edge,
                                     StringID tag,
                                     Allocator &allocator);
    public:
        IndexManager(const uint64_t region_addr, bool create)
            : _tag_prop_map(reinterpret_cast<TagList *>(region_addr)),
                _create(create), _initialized(false)
        {
            _tag_prop_map[0].init();
            _tag_prop_map[1].init();
        }

        void init(Allocator &allocator)
        {
            if (_create) {
                // Create the tag=0 entry already for situation where a user
                // does not want to tag nodes/edges but don't allow a default
                // indexing on tag value of 0 like other cases since an
                // untagged node/edge implies searching the entire table anyway
                // without a property index
                _tag_prop_map[0].add(0, allocator);
                _tag_prop_map[1].add(0, allocator);
            }
            _initialized = true;
        }

        void create_index(int node_or_edge, StringID tag,
                            StringID property_id,
                            PropertyType ptype,
                            Allocator &allocator);

        // Nodes and edges will have to be added to an index in two
        // stages. One at the add_node or add_edge stage. Another at
        // the set_property state. We cannot wait until first set_property
        // since the user might just want to query based on tag and
        // not set any property until then.
        bool add_node(Node *n, Allocator &allocator);

        // This is called from set_property
        bool add_node(StringID property_id, const Property &p,
                        Node *n, Allocator &allocator);

        Index *get_index(StringID tag, const PropertyPredicate &pp);
        NodeIterator get_nodes(StringID tag);
    };
}
