#pragma once

#include <locale>
#include <stddef.h>
#include "graph.h"
#include "Allocator.h"
#include "IndexManager.h"
#include "TransactionManager.h"
#include "os.h"
#include "StringTable.h"

namespace Jarvis {
    struct RegionInfo;

    class GraphImpl {
        typedef FixedAllocator NodeTable;
        typedef FixedAllocator EdgeTable;

        struct GraphInfo;

        struct GraphInit {
            bool create;
            bool read_only;
            unsigned node_size;
            unsigned edge_size;

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

    public:
        GraphImpl(const char *name, int options, const Graph::Config *config);
        TransactionManager &transaction_manager() { return _transaction_manager; }
        IndexManager &index_manager() { return _index_manager; }
        StringTable &string_table() { return _string_table; }
        NodeTable &node_table() { return _node_table; }
        EdgeTable &edge_table() { return _edge_table; }
        Allocator &allocator() { return _allocator; }
        std::locale &locale() { return _locale; }

        void check_read_write()
        {
            if (_init.read_only)
                throw Exception(ReadOnly);
        }
    };
};
