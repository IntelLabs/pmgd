/**
 * @file   EdgeIndex.cc
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

#include "EdgeIndex.h"
#include "exception.h"

using namespace PMGD;

namespace PMGD {
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

    void EdgeIndex::remove(StringID key, Edge* edge, Allocator &allocator)
    {
        // Construct the entry that gets removed from the list
        EdgeIndexType newkey(key);
        // Create the pair that gets removed from the node-edge list
        KeyValuePair<Edge *, Node *> addrs(edge, 0);

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
