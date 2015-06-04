#pragma once
#include "stringid.h"
#include "ChunkList.h"
#include "property.h"
#include "Index.h"
#include "iterator.h"

namespace Jarvis {
    class Node;
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

        IndexList *add_tag_index(Graph::IndexType index_type,
                                     StringID tag,
                                     Allocator &allocator);
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
        bool add_node(Node *n, Allocator &allocator);

        Index *get_index(Graph::IndexType index_type, StringID tag,
                         StringID property_id,
                         PropertyType ptype = PropertyType(0));
        NodeIterator get_nodes(StringID tag);
    };
}
