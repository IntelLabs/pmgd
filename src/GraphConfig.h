#include <string>
#include <vector>

#include "graph.h"
#include "os.h"

namespace Jarvis {
    struct RegionInfo {
        static const int REGION_NAME_LEN = 32;
        char name[REGION_NAME_LEN];
        uint64_t addr;
        size_t len;
    };

    struct AllocatorInfo {
        unsigned object_size;
        uint64_t offset;
        uint64_t pool_size;
    };

    struct GraphConfig {
        static const size_t BASE_ADDRESS = SIZE_1TB;
        static const size_t INFO_SIZE = SIZE_4KB;

        unsigned node_size;
        unsigned edge_size;
        unsigned max_stringid_length;
        std::string locale_name;

        RegionInfo transaction_info;
        RegionInfo journal_info;
        RegionInfo indexmanager_info;
        RegionInfo stringtable_info;
        RegionInfo node_info;
        RegionInfo edge_info;
        RegionInfo allocator_info;

        std::vector<AllocatorInfo> fixed_allocator_info;

        GraphConfig(const Graph::Config *user_config);
        void init_region_info(RegionInfo &info, const char *name,
                              uint64_t &addr, size_t size);
        void add_allocator(unsigned object_size, uint64_t &offset, size_t size);
    };
};
