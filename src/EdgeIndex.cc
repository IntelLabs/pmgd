#include "EdgeIndex.h"
#include "exception.h"

using namespace Jarvis;

namespace Jarvis {
    void EdgeIndex::add(StringID key, Edge* edge, Node* node,
            FixedAllocator &allocator)
    {
        EdgeIndexType newkey(key);
        // Create a pair to add in the node-edge list
        KeyValuePair<Edge *, Node *> addrs(edge, node);

        // Construct the entry that goes in the list
        // starting with the list portion unless an entry for this
        // key already exists
        EdgeIndexType *ptr = _key_list.find(newkey);
        if (ptr != NULL) {
            ptr->add(addrs, allocator);
            return;
        }

        // First create an entry for this key with the simple constructor
        // This could have been a pointer and then init would be easy
        // but then we would have all sorts of indirections that we want
        // to avoid. So take a copy hit here with temporary objects. These
        // objects are small anyway
        // EdgeIndexType ptr
        ptr = _key_list.add(newkey, allocator);
        // TODO log
        // Now that you have the PM location, add all the values
        ptr->init(key, addrs, allocator);
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
            FixedAllocator &allocator)
    {
        EdgeIndexType newkey(key);
        // Create a pair to add in the node-edge list
        KeyValuePair<Edge *, Node *> addrs(edge, node);

        // Construct the entry that gets removed from the list
        EdgeIndexType *ptr = _key_list.find(newkey);
        if (ptr != NULL) {
            // First remove from internal data structure
            ptr->remove(addrs, allocator);
            // If no more edges in that tag bucket, remove the bucket
            if (ptr->num_elems() == 0)
                _key_list.remove(newkey, allocator);
            return;
        }
    }
}
