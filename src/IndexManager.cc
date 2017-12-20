/**
 * @file   IndexManager.cc
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

#include <assert.h>
#include "IndexManager.h"
#include "graph.h"
#include "List.h"
#include "AvlTreeIndex.h"

using namespace PMGD;

IndexManager::IndexList *IndexManager::add_tag_index(
                                           Graph::IndexType index_type,
                                           StringID tag,
                                           Allocator &allocator)
{
    // ChunkList add encapsulates locking. That means, if the tag didn't
    // exist and is added to an existing chunk, then that entire chunk
    // is write locked till TX finishes. Otherwise the relevant chunk is
    // read locked.
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
                throw PMGDException(NotImplemented);
            case PropertyType::Blob:
                throw PMGDException(PropertyTypeInvalid);
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
    List<void *> *list = idx->find(value, true);
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

Graph::IndexStats IndexManager::get_index_stats(Graph::IndexType index_type, StringID tag,
                               StringID property_id)
{
    Index *index = get_index(index_type, tag, property_id);

    if (!index) {
        Graph::IndexStats stats = {0,0,0,0,0};
        return stats;
    }

    return index->get_stats();
}

 Graph::IndexStats IndexManager::get_index_stats(Graph::IndexType index_type, StringID tag)
{
    IndexList *tag_entry = _tag_prop_map[index_type].find(tag);

    return get_index_stats(tag_entry);
}

Graph::IndexStats IndexManager::get_index_stats(Graph::IndexType index_type)
{
    Graph::IndexStats stats = {0,0,0,0,0};

    std::vector<KeyValuePair<StringID,IndexList> *> indexes_vector =
                                _tag_prop_map[index_type].get_key_values();

    // For health calculation
    std::vector<Graph::IndexStats> tag_level_stats;

    for (auto& idx_list: indexes_vector) {
        Graph::IndexStats idx_stats = get_index_stats(&idx_list->value());
        tag_level_stats.push_back(idx_stats);
        stats.total_size_bytes     += idx_stats.total_size_bytes;
        stats.total_elements       += idx_stats.total_elements;
        stats.total_unique_entries += idx_stats.total_unique_entries;
    }

    if (stats.total_elements == 0)
        stats.health_factor = 100;
    else {
        for (auto& tag_stats : tag_level_stats) {
            stats.health_factor += tag_stats.health_factor *
                                   tag_stats.total_elements /
                                   stats.total_elements;
        }
    }

    return stats;
}

Graph::IndexStats IndexManager::get_index_stats(IndexList *tag_entry)
{
    Graph::IndexStats stats = {0,0,0,0,0};

    // Check first if that tag index exists.
    if (!tag_entry)
        return stats;

    std::vector<KeyValuePair<StringID,Index *> *> indexes = tag_entry->get_key_values();

    // For health calculation
    std::vector<Graph::IndexStats> property_level_stats;

    size_t total_elements_health = 0;

    for (auto& idx: indexes) {
        Graph::IndexStats idx_stats = idx->value()->get_stats();
        stats.total_size_bytes     += idx_stats.total_size_bytes;
        stats.total_elements       += idx_stats.total_elements;
        stats.total_unique_entries += idx_stats.total_unique_entries;
        if (idx->key() != StringID(0)) { // Not used in health calculation
            property_level_stats.push_back(idx_stats);
            total_elements_health  += idx_stats.total_elements;
        }
    }

    if (total_elements_health == 0)
        stats.health_factor = 100;
    else {
        for (auto& property_stats : property_level_stats) {
            stats.health_factor += property_stats.health_factor *
                                   property_stats.total_elements /
                                   total_elements_health;
        }
    }

    return stats;
}

Graph::IndexStats IndexManager::get_index_stats()
{
    Graph::IndexStats stats_nodes = get_index_stats(Graph::NodeIndex);
    Graph::IndexStats stats_edges = get_index_stats(Graph::EdgeIndex);

    Graph::IndexStats stats = {0,0,0,0,0};

    stats.total_unique_entries = stats_nodes.total_unique_entries +
                                 stats_edges.total_unique_entries;
    stats.total_elements       = stats_nodes.total_elements +
                                 stats_edges.total_elements;
    stats.total_size_bytes     = stats_nodes.total_size_bytes +
                                 stats_edges.total_size_bytes;

    if (stats.total_elements == 0)
        stats.health_factor = 100;
    else {
        stats.health_factor += stats_nodes.total_elements *
                               stats_nodes.health_factor /
                               stats.total_elements;
        stats.health_factor += stats_edges.total_elements *
                               stats_edges.health_factor /
                               stats.total_elements;
    }

    return stats;
}

Graph::ChunkStats IndexManager::get_all_chunk_lists_stats()
{
    Graph::ChunkStats stats = {0,0,0,0,0};

    std::vector<size_t> num_elements_vector;
    std::vector<size_t> health_vector;

    for (unsigned i = 0; i < 2; ++i) {
        TagList tag_list = _tag_prop_map[i];

        stats.total_chunks     += tag_list.total_chunks();
        stats.num_elements     += tag_list.num_elems();
        stats.total_size_bytes += tag_list.chunk_list_size();
        num_elements_vector.push_back(tag_list.num_elems());
        health_vector      .push_back(tag_list.health());

        std::vector<KeyValuePair<StringID,IndexList> *> chunk_vector =
                                _tag_prop_map[i].get_key_values();

        for (auto& chunk: chunk_vector) {
            stats.total_chunks     += chunk->value().total_chunks();
            stats.num_elements     += chunk->value().num_elems();
            stats.total_size_bytes += chunk->value().chunk_list_size();
            num_elements_vector.push_back(chunk->value().num_elems());
            health_vector      .push_back(chunk->value().health());
        }
    }

    if (stats.num_elements == 0)
        stats.health_factor = 100;
    else {
        for (size_t i = 0; i < health_vector.size(); ++i) {
            stats.health_factor += health_vector.at(i) *
                                   num_elements_vector.at(i) /
                                   stats.num_elements;
        }
    }

    return stats;
}

Graph::ChunkStats IndexManager::get_chunk_list_stats(Graph::IndexType index_type)
{
    Graph::ChunkStats stats = {0,0,0,0,0};

    TagList tag_list = _tag_prop_map[index_type];

    // Stats object is created and populated here to avoid
    // making the ChunkList aware of structs defined in graph.h
    // This will cause total_chunks() being executed multiple
    // times, thus traversing the ChunkList multiple times.
    // Since the ChunkList have small number of chunks,
    // we prefered this approach to avoid moving this logic
    // into the ChunkList.

    stats.total_chunks     = tag_list.total_chunks();
    stats.chunk_size       = tag_list.chunk_size_bytes();
    stats.num_elements     = tag_list.num_elems();
    stats.total_size_bytes = tag_list.chunk_list_size();
    stats.health_factor    = tag_list.health();

    return stats;
}

Graph::ChunkStats IndexManager::get_chunk_list_stats(Graph::IndexType index_type, StringID tag)
{
    Graph::ChunkStats stats = {0,0,0,0,0};

    IndexList *tag_entry = _tag_prop_map[index_type].find(tag);

    if (!tag_entry)
        return stats;

    stats.total_chunks     = tag_entry->total_chunks();
    stats.chunk_size       = tag_entry->chunk_size_bytes();
    stats.num_elements     = tag_entry->num_elems();
    stats.total_size_bytes = tag_entry->chunk_list_size();
    stats.health_factor    = tag_entry->health();

    return stats;
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
    return prop0_idx->get_iterator(index_type, PropertyPredicate(0, PropertyPredicate::Eq, true),
                                    NULL, false);
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
