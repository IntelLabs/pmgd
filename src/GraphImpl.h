/**
 * @file   GraphImpl.h
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

#pragma once

#include <locale>
#include <stddef.h>
#include "graph.h"
#include "GraphConfig.h"
#include "Allocator.h"
#include "IndexManager.h"
#include "TransactionManager.h"
#include "os.h"
#include "StringTable.h"
#include "lock.h"

namespace PMGD {
    struct RegionInfo;

    class GraphImpl {
    public:
        typedef FixedAllocator NodeTable;
        typedef FixedAllocator EdgeTable;

    private:
        struct GraphInfo;

        struct GraphInit {
            unsigned node_size;
            unsigned edge_size;
            unsigned num_allocators;

            CommonParams params;

            // Lock stripes can be created with different sizes at
            // each use of the graph. Hence, not stored in PM.
            size_t node_striped_lock_size;    // bytes
            size_t edge_striped_lock_size;    // bytes
            size_t index_striped_lock_size;   // bytes

            // Same reason to not store in PM.
            unsigned node_stripe_width;
            unsigned edge_stripe_width;
            unsigned index_stripe_width;

            os::MapRegion info_map;
            GraphInfo *info;

            GraphInit(const char *name, int options, const Graph::Config *);
        };

        class MapRegion : public os::MapRegion {
        public:
            MapRegion(const char *db_name, const RegionInfo &info, bool create, bool read_only);
        };

        // Order here is important: SigHandler must be first,
        // followed by GraphInit.
        os::SigHandler _sighandler;
        GraphInit _init;

        // File-backed space
        MapRegion _transaction_region;
        MapRegion _journal_region;
        MapRegion _indexmanager_region;
        MapRegion _stringtable_region;
        MapRegion _node_region;
        MapRegion _edge_region;
        MapRegion _allocator_region;

        // TransactionManager needs be first to do recovery.
        TransactionManager _transaction_manager;
        IndexManager _index_manager;
        StringTable _string_table;
        NodeTable _node_table;
        EdgeTable _edge_table;
        Allocator _allocator;

        std::locale _locale;

        // Locks for various components.
        StripedLock _node_locks;
        StripedLock _edge_locks;
        StripedLock _index_locks;

    public:
        GraphImpl(const char *name, int options, const Graph::Config *config);
        TransactionManager &transaction_manager() { return _transaction_manager; }
        IndexManager &index_manager() { return _index_manager; }
        StringTable &string_table() { return _string_table; }
        NodeTable &node_table() { return _node_table; }
        EdgeTable &edge_table() { return _edge_table; }
        Allocator &allocator() { return _allocator; }
        std::locale &locale() { return _locale; }
        StripedLock &node_locks() { return _node_locks; }
        StripedLock &edge_locks() { return _edge_locks; }
        StripedLock &index_locks() { return _index_locks; }

        void check_read_write()
        {
            if (_init.params.read_only)
                throw PMGDException(ReadOnly);
        }

        void msync_options(bool &msync_needed, bool &always_msync)
        {
            msync_needed = _init.params.msync_needed;
            always_msync = _init.params.always_msync;
        }
    };
};
