#include <assert.h>
#include "IndexManager.h"
#include "graph.h"
#include "List.h"
#include "AvlTreeIndex.h"

using namespace Jarvis;

IndexManager::IndexList *IndexManager::add_tag_index(
                                           Graph::IndexType index_type,
                                           StringID tag,
                                           Allocator &allocator)
{
    // ChunkList should call the empty constructor if this element was
    // newly added. So the entry will always be correctly initialized
    IndexList *tag_entry = _tag_prop_map[index_type].add(tag, allocator);
    // If there is no other property id entry for this tag, make that
    // 0th entry be an entry for NO property id (represented by 0). So
    // all nodes/edges for this tag will get indexed here apart from their
    // indexed property values.
    if (tag_entry->num_elems() == 0 && !(tag == 0)) {
        // All we need is one value in the tree. Hopefully we will have different
        // type of data structure options eventually.
        Index *prop0_idx = new (allocator.alloc(sizeof(BoolValueIndex))) BoolValueIndex(PropertyType::Boolean);
        Index **value = tag_entry->add(0, allocator);
        *value = prop0_idx;
    }

    return tag_entry;
}

// The general order of data structures is:
// IndexManager->_tag_prop_map[node/edge]->_propid_propvalueadt_map->the index
void IndexManager::create_index(Graph::IndexType index_type, StringID tag,
                                StringID property_id,
                                PropertyType ptype,
                                Allocator &allocator)
{
    // Check if there is an entry for this tag. If there is,
    // there will already be a property id data structure there,
    // which will get returned to us and then we can add a new
    // property ID. If not, this add_tag_index function will
    // allocate it and create an entry with id 0 for default (for tag!=0).
    IndexList *tag_entry = add_tag_index(index_type, tag, allocator);

    // The goal is to create an index for property type ptype at the given
    // (tag,propid) combination for node or edge
    Index **prop_idx = tag_entry->add(property_id, allocator);

    if (*prop_idx == NULL) {
        switch(ptype) {
            case PropertyType::Integer:
                *prop_idx = new (allocator.alloc(sizeof(LongValueIndex))) LongValueIndex(ptype);
                break;
            case PropertyType::Float:
                *prop_idx = new (allocator.alloc(sizeof(FloatValueIndex))) FloatValueIndex(ptype);
                break;
            case PropertyType::Boolean:
                *prop_idx = new (allocator.alloc(sizeof(BoolValueIndex))) BoolValueIndex(ptype);
                break;
            case PropertyType::Time:
                *prop_idx = new (allocator.alloc(sizeof(TimeValueIndex))) TimeValueIndex(ptype);
                break;
            case PropertyType::String:
                *prop_idx = new (allocator.alloc(sizeof(StringValueIndex))) StringValueIndex(ptype);
                break;
            case PropertyType::NoValue:
                throw Exception(NotImplemented);
            case PropertyType::Blob:
                throw Exception(PropertyTypeInvalid);
        }
    }
}

bool IndexManager::add(Graph::IndexType index_type, StringID tag, void *obj,
                       Allocator &allocator)
{
    assert(obj != NULL);

    if (tag == 0)
        return false;

    // Check first if that tag index exists. Since we are indexing
    // all nodes/edges based on their tags, create an entry if it
    // doesn't exist.
    IndexList *tag_entry = add_tag_index(index_type, tag, allocator);

    // For now, add only to the no property list ==> index via tag
    // This entry should always exist since we add it explicitly when
    // creating a new tag entry
    // TODO: Perhaps use NoValue in this case instead of Boolean.
    // TODO: Tree is unnecessary and Node* needs better arrangement for
    // quick search and remove operations
    BoolValueIndex *idx = (BoolValueIndex *)*(tag_entry->find(0));

    // Now retrieve the list where node pointers are getting added
    // This particular property only has one value = true and should
    // be the first and only node in the tree.
    bool value = true;
    List<void *> *list = idx->add(value, allocator);
    list->add(obj, allocator);

    return true;
}

void IndexManager::remove(Graph::IndexType index_type, StringID tag, void *obj,
                          Allocator &allocator)
{
    assert(obj != NULL);

    if (tag == 0)
        return;

    // Get the tag index. Since we are indexing all nodes based
    // on their tags, it should always exist.
    IndexList *tag_entry = _tag_prop_map[index_type].add(tag, allocator);

    // Get the no property list ==> index via tag
    // This entry should always exist.
    BoolValueIndex *idx = (BoolValueIndex *)*(tag_entry->find(0));

    // Now retrieve the list where node pointers are added
    // This particular property only has one value = true and should
    // be the first and only node in the tree.
    bool value = true;
    List<void *> *list = idx->find(value);
    list->remove(obj, allocator);
}

Index *IndexManager::get_index(Graph::IndexType index_type, StringID tag,
                               StringID property_id, PropertyType ptype)
{
    // Traverse the two chunklists <tag,propADT> and <propid,indexADT>
    // to get to the Index* which is where this Node* needs to be added
    IndexList *tag_entry = _tag_prop_map[index_type].find(tag);
    // Check first if that tag index exists. Since we add every new tag
    // to our tag index, this shouldn't happen.
    if (!tag_entry)
        return NULL;

    // Now get the generic pointer and let Index class handle the specific
    // property datatype.
    // If property had not been indexed, this will be null.
    Index **idx = tag_entry->find(property_id);

    if (idx == NULL)
        return NULL;
    // This call throws if the types don't match for an
    // existing index.
    if (ptype != PropertyType(0))
        (*idx)->check_type(ptype);
    return *idx;
}

Index::Index_IteratorImplIntf *IndexManager::get_iterator
    (Graph::IndexType index_type, StringID tag)
{
    Index *prop0_idx;
    prop0_idx = get_index(index_type, tag, 0);
    if (!prop0_idx)
        return NULL;
    // This index can never be null cause we create it for each non-zero tag.
    // The second parameter here is the locale which we surely do not need for
    // a boolean property.
    return prop0_idx->get_iterator(PropertyPredicate(0, PropertyPredicate::Eq, true), NULL, false);
}

void IndexManager::update
    (GraphImpl *db, Graph::IndexType index_type, StringID tag, void *obj,
     StringID id, const PropertyRef *old_value, const Property *new_value)
{
    // get_index throws if the property type doesn't match the index.
    PropertyType ptype = new_value ? new_value->type() : PropertyType(0);
    Index *index = get_index(index_type, tag, id, ptype);

    // Check if there is an index with this property id and tag = 0.
    // This is a general all-tag index for certain properties such as loader id.
    Index *gindex = get_index(Graph::IndexType(index_type), 0, id, ptype);

    if (old_value != NULL && (index != NULL || gindex != NULL)) {
        Property tmp(*old_value);
        if (index)
            index->remove(tmp, obj, db);
        if (gindex)
            gindex->remove(tmp, obj, db);
    }

    if (new_value != NULL) {
        if (index)
            index->add(*new_value, obj, db);
        if (gindex)
            gindex->add(*new_value, obj, db);
    }
}
