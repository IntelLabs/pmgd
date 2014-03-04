#pragma once

#include <stddef.h>
#include "graph.h"
#include "allocator.h"
#include "TransactionManager.h"
#include "os.h"

namespace Jarvis {
    class GraphImpl {
        typedef FixedAllocator NodeTable;
        typedef FixedAllocator EdgeTable;

        struct RegionInfo;
        struct GraphInfo;

        struct GraphInit {
            bool create;
            os::MapRegion info_map;
            GraphInfo *info;

            GraphInit(const char *name, int options);
        };

        class MapRegion : public os::MapRegion {
        public:
            MapRegion(const char *db_name, const RegionInfo &info, bool create);
        };

        static const RegionInfo default_regions[];
        static const AllocatorInfo default_allocators[];
        static const size_t NUM_FIXED_ALLOCATORS;

        // ** Order here is important: GraphInit MUST be first
        GraphInit _init;

        // File-backed space
        MapRegion _transaction_region;
        MapRegion _node_region;
        MapRegion _edge_region;
        MapRegion _allocator_region;

        TransactionManager _transaction_manager;
        NodeTable _node_table;
        EdgeTable _edge_table;
        Allocator _allocator;

        AllocatorInfo allocator_info(const RegionInfo &info,
                                     uint32_t obj_size) const;

    public:
        GraphImpl(const char *name, int options);
        TransactionManager &transaction_manager() { return _transaction_manager; }
        NodeTable &node_table() { return _node_table; }
        EdgeTable &edge_table() { return _edge_table; }
        Allocator &allocator() { return _allocator; }
    };
};
