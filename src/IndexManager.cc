#include <assert.h>
#include "IndexManager.h"
#include "graph.h"
#include "List.h"
#include "AvlTreeIndex.h"

using namespace Jarvis;

// For the actual property value indices
typedef AvlTreeIndex<long long,List<Node *>> NodeIntIndex;
typedef AvlTreeIndex<double,List<Node *>> NodeFloatIndex;
typedef AvlTreeIndex<bool,List<Node *>> NodeBoolIndex;

IndexManager::IndexList *IndexManager::add_tag_index(int node_or_edge,
                                           StringID tag,
                                           Allocator &allocator)
{
    // ChunkList should call the empty constructor if this element was
    // newly added. So the entry will always be correctly initialized
    IndexList *tag_entry = _tag_prop_map[node_or_edge - 1].add(tag, allocator);
    // If there is no other property id entry for this tag, make that
    // 0th entry be an entry for NO property id (represented by 0). So
    // all nodes/edges for this tag will get indexed here apart from their
    // indexed property values.
    if (tag_entry->num_elems() == 0 && !(tag == 0)) {
        // All we need is one value in the tree. Hopefully we will have different
        // type of data structure options eventually.
        Index *prop0_idx = (Index *)allocator.alloc(sizeof(NodeBoolIndex));
        prop0_idx->init(t_boolean);
        Index **value = tag_entry->add(0, allocator);
        *value = prop0_idx;
    }

    return tag_entry;
}

// The general order of data structures is:
// IndexManager->_tag_prop_map[node/edge]->_propid_propvalueadt_map->the index
void IndexManager::create_index(int node_or_edge, StringID tag,
                                StringID property_id,
                                PropertyType ptype,
                                Allocator &allocator)
{
    // Check if there is an entry for this tag. If there is,
    // there will already be a property id data structure there,
    // which will get returned to us and then we can add a new
    // property ID. If not, this add_tag_index function will
    // allocate it and create an entry with id 0 for default (for tag!=0).
    IndexList *tag_entry = add_tag_index(node_or_edge, tag, allocator);

    // The goal is to create an index for property type ptype at the given
    // (tag,propid) combination for node or edge
    Index **prop_idx = tag_entry->add(property_id, allocator);

    if (*prop_idx == NULL) {
        switch(ptype) {
            case t_integer:
                *prop_idx = (Index *)allocator.alloc(sizeof(NodeIntIndex));
                (*prop_idx)->init(t_integer);
                break;
            case t_float:
                *prop_idx = (Index *)allocator.alloc(sizeof(NodeFloatIndex));
                (*prop_idx)->init(t_float);
                break;
            case t_boolean:
                *prop_idx = (Index *)allocator.alloc(sizeof(NodeBoolIndex));
                (*prop_idx)->init(t_boolean);
                break;
            case t_time:
            case t_string:
            case t_novalue:
                throw Exception(not_implemented);
            case t_blob:
                throw Exception(property_type);
        }
    }
}

bool IndexManager::add_node(Node *n, Allocator &allocator)
{
    assert(n != NULL);
 
    if (n->get_tag() == 0)
        return false;

    // Check first if that tag index exists. Since we are indexing
    // all nodes/edges based on their tags, create an entry if it
    // doesn't exist.
    IndexList *tag_entry = add_tag_index(Graph::NODE, n->get_tag(), allocator);
 
    // For now, add only to the no property list ==> index via tag
    // This entry should always exist since we add it explicitly when
    // creating a new tag entry
    // TODO: Perhaps use t_novalue in this case instead of boolean.
    // TODO: Tree is unnecessary and Node* needs better arrangement for
    // quick search and remove operations
    NodeBoolIndex *idx = (NodeBoolIndex *)*(tag_entry->find(0));
    // Now retrieve the list where node pointers are getting added
    // This particular property only has one value = true and should
    // be the first and only node in the tree.
    bool value = true;
    List<Node *> *list = idx->add(value, allocator);
    list->add(n, allocator);

    return true;
}

Index *IndexManager::get_index(int node_or_edge, StringID tag, StringID property_id)
{
    // Traverse the two chunklists <tag,propADT> and <propid,indexADT>
    // to get to the Index* which is where this Node* needs to be added
    IndexList *tag_entry = _tag_prop_map[node_or_edge - 1].find(tag);
    // Check first if that tag index exists. Since we add every new tag
    // to our tag index, this shouldn't happen.
    if (!tag_entry)
        return NULL;

    // Now get the generic pointer and let Index class handle the specific
    // property datatype.
    // If property had not been indexed, this will be null.
    Index **idx = tag_entry->find(property_id);

    return (idx != NULL) ? *idx : NULL;
}

NodeIterator IndexManager::get_nodes(StringID tag)
{
    Index *prop0_idx;
    prop0_idx = get_index(Graph::NODE, tag, 0);
    if (!prop0_idx)
        return NodeIterator(NULL);
    // This index can never be null cause we create it for each non-zero tag
    return prop0_idx->get_nodes(PropertyPredicate(0, PropertyPredicate::eq, true));
}
