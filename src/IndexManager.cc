#include <assert.h>
#include "IndexManager.h"

using namespace Jarvis;

IndexManager::PropertyIdADT *IndexManager::add_tag_index(int node_or_edge,
                                           StringID tag,
                                           Allocator &allocator)
{
    // TODO Check if this is even needed
    if (!_initialized)
        init(allocator);
    PropertyIdADT *tag_entry = _tag_prop_map[node_or_edge - 1].add(tag, allocator);
    tag_entry->init();
    // If there is no other property id entry for this tag, make that
    // 0th entry be an entry for NO property id (represented by 0). So
    // all nodes/edges for this tag will get indexed here apart from their
    // indexed property values.
    if (tag_entry->num_elems() == 0) {
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
    // there will already be a property id data structure there
    // which will get returned to us and then we can add a new
    // property ID there. If not, this add_tag_index function will
    // allocate it and create an entry with id 0 for default.
    PropertyIdADT *tag_entry = _tag_prop_map[node_or_edge - 1].find(tag);
    if (!tag_entry)
        tag_entry = add_tag_index(node_or_edge, tag, allocator);

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
                break;
            case t_novalue:
                break;
            case t_time:
            case t_boolean:
            case t_string:
                throw Exception(not_implemented);
            case t_blob:
                throw Exception(property_type);
        }
    }
}

bool IndexManager::add_node(const Node *n, Allocator &allocator)
{
    assert(n != NULL);
    // Untagged nodes can be indexed only because of properties. Not
    // in the default tag index.
    if (n->get_tag() == 0)
        return false;
    // Traverse the two chunklists <tag,propADT> and <propid,indexADT>
    // to get to the Index* which is where this Node* needs to be added
    PropertyIdADT *tag_entry = _tag_prop_map[0].find(n->get_tag());
    // Check first if that tag index exists. Since we are indexing
    // all nodes/edges based on their tags, create an entry if it
    // doesn't exist.
    if (!tag_entry)
        tag_entry = add_tag_index(0, n->get_tag(), allocator);
 
    // For now, add only to the no property list ==> index via tag
    // This entry should always exist since we add it explicitly when
    // creating a new tag entry
    NodeBoolIndex *idx = (NodeBoolIndex *)*(tag_entry->find(0));
    // Now retrieve the list where node pointers are getting added
    // This particular property only has one value = true and should
    // be the first and only node in the tree.
    // TODO: Tree is unnecessary and Node* needs better arrangement for
    // quick search and remove operations
    bool value = true;
    List<const Node *> *list = idx->add(value, allocator);
    list->add(n, allocator);

    return true;
}

bool IndexManager::add_node(StringID property_id, const Property &p,
                            const Node *n, Allocator &allocator)
{
    // This might be quite redundant. This function is called from
    // node.set_property() and it is very unlikely that n will be null.
    assert(n != NULL);
    // Traverse the two chunklists <tag,propADT> and <propid,indexADT>
    // to get to the Index* which is where this Node* needs to be added
    PropertyIdADT *tag_entry = _tag_prop_map[0].find(n->get_tag());
    // Check first if that tag index exists. Since we add every new tag
    // to our tag index, this shouldn't happen.
    if (!tag_entry)
        throw Exception(invalid_id);

    // Now get the generic pointer and let Index class handle the specific
    // property datatype.
    Index *idx = (Index *)*(tag_entry->find(property_id));
    // If property had not been indexed, this will be null.
    if (!idx)
        return false;
    idx->add(p, n, allocator);
    
    return true;
}
