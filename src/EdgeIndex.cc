#include "EdgeIndex.h"
#include "exception.h"

using namespace Jarvis;

namespace Jarvis {
    void EdgeIndex::add(StringID key, Edge* edge, Node* node,
            Allocator &allocator)
    {
        // Temporary DRAM object for searching the key data structure
        EdgeIndexType newkey(key);
        // Create a pair to add in the node-edge list
        KeyValuePair<Edge *, Node *> addrs(edge, node);

        // Search for the key in first data structure. The address pair
        // goes in the second data structure which serves as value for
        // the given key
        EdgeIndexType *ptr = _key_list.find(newkey);
        if (ptr == NULL) {  // Seeing this key for the first time
            // First create an entry for this key with the simple constructor
            // EdgeIndexType ptr
            ptr = _key_list.add(newkey, allocator);
            // Initialize the second data structure mapped to 'key'
            ptr->init(key, allocator);
        }

        // Logging done inside add
        ptr->add(addrs, allocator);
    }

    const EdgeIndex::EdgePosition *EdgeIndex::get_first(StringID key)
    {
        EdgeIndexType newkey(key);
        // Construct the entry for search in the list
        // If they key is found, there should at least be one element
        // in the list
        EdgeIndexType *ptr = _key_list.find(newkey);
        if (ptr != NULL)
            return ptr->get_first();
        return NULL;
    }

    void EdgeIndex::remove(StringID key, Edge* edge, Node* node, 
            Allocator &allocator)
    {
        // Construct the entry that gets removed from the list
        EdgeIndexType newkey(key);
        // Create the pair that gets removed from the node-edge list
        KeyValuePair<Edge *, Node *> addrs(edge, node);

        EdgeIndexType *ptr = _key_list.find(newkey);
        // If key found
        if (ptr != NULL) {
            // Logging for both the steps handled in the remove function
            // First remove from internal data structure
            ptr->remove(addrs, allocator);
            // If no more edges in that tag bucket, remove the bucket
            if (ptr->num_elems() == 0)
                _key_list.remove(newkey, allocator);
            return;
        }
    }
}
