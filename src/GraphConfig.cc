#include <stddef.h>
#include <string.h>
#include <locale.h>
#include "graph.h"
#include "GraphConfig.h"
#include "GraphImpl.h"
#include "Allocator.h"

using namespace Jarvis;

static const size_t DEFAULT_NODE_SIZE = 64;
static const size_t DEFAULT_EDGE_SIZE = 32;
static const size_t DEFAULT_NUM_FIXED_ALLOCATORS = 5;

// The object size of the smallest fixed allocator.
static const size_t MIN_FIXED_ALLOCATOR = 16;

static const size_t DEFAULT_TRANSACTION_TABLE_SIZE = SIZE_4KB;
static const size_t DEFAULT_JOURNAL_SIZE = 64 * SIZE_2MB;

static const size_t INDEX_MANAGER_SIZE = SIZE_4KB;

static const int DEFAULT_MAX_STRINGID_LENGTH = 16;
static const int DEFAULT_MAX_STRINGIDS = 4096;
static const size_t DEFAULT_STRING_TABLE_SIZE = DEFAULT_MAX_STRINGIDS * DEFAULT_MAX_STRINGID_LENGTH;

static inline size_t align(size_t addr, size_t alignment)
{
    return (addr + alignment - 1) & ~(alignment - 1);
}


Graph::Config::Config()
{
    memset(this, 0, offsetof(Config, locale_name));
}


template <typename T>
void check_power_of_two(T val)
{
    if ((val & (val - 1)) != 0)
        throw Exception(InvalidConfig);
}

GraphConfig::GraphConfig(const Graph::Config *user_config)
{
#define VALUE(field, default) \
            (user_config != NULL && user_config->field != 0 \
                ? user_config->field : (default))

    size_t default_region_size = VALUE(default_region_size,os::get_default_region_size());

    node_size = VALUE(node_size, DEFAULT_NODE_SIZE);
    if (node_size < DEFAULT_NODE_SIZE)
        throw Exception(InvalidConfig);
    check_power_of_two(node_size);

    edge_size = VALUE(edge_size, DEFAULT_EDGE_SIZE);
    if (edge_size < DEFAULT_EDGE_SIZE)
        throw Exception(InvalidConfig);
    check_power_of_two(edge_size);

    max_stringid_length = VALUE(max_stringid_length, DEFAULT_MAX_STRINGID_LENGTH);
    check_power_of_two(max_stringid_length);

    size_t transaction_table_size = VALUE(transaction_table_size, DEFAULT_TRANSACTION_TABLE_SIZE);
    size_t journal_size = VALUE(journal_size, DEFAULT_JOURNAL_SIZE);

    locale_name = user_config != NULL && !user_config->locale_name.empty()
                      ? user_config->locale_name : std::locale().name();

    size_t node_table_size = VALUE(node_table_size, default_region_size);
    if (node_table_size % node_size != 0)
        throw Exception(InvalidConfig);

    size_t edge_table_size = VALUE(edge_table_size, default_region_size);
    if (edge_table_size % edge_size != 0)
        throw Exception(InvalidConfig);

    size_t string_table_size = VALUE(string_table_size, DEFAULT_STRING_TABLE_SIZE);
    check_power_of_two(string_table_size);

    // 'Addr' is updated by init_region_info to the end of the region,
    // so it can be used to determine the base address of the next region.
    uint64_t addr = BASE_ADDRESS + INFO_SIZE;
    init_region_info(indexmanager_info, "indexmanager.jdb", addr,
         INDEX_MANAGER_SIZE);
    init_region_info(stringtable_info, "stringtable.jdb", addr,
        string_table_size);
    init_region_info(transaction_info, "transaction.jdb", addr,
         transaction_table_size);
    init_region_info(journal_info, "journal.jdb", addr, journal_size);
    init_region_info(node_info, "nodes.jdb", addr, node_table_size);
    init_region_info(edge_info, "edges.jdb", addr, edge_table_size);


    // 'Offset' is updated by add_allocator to the end of the pool,
    // so it can be used to determine the offset of the next pool.
    size_t offset = 0;

    if (user_config != NULL && user_config->fixed_allocators.size() != 0) {
        if (user_config->fixed_allocators.size() > GraphImpl::MAX_FIXED_ALLOCATORS)
            throw Exception(InvalidConfig);

        unsigned next_size = MIN_FIXED_ALLOCATOR;
        for (auto a : user_config->fixed_allocators) {
            unsigned object_size = a.object_size;
            size_t allocator_size = a.size;

            if (object_size < next_size
                    || (object_size & (object_size - 1)) != 0
                    || (allocator_size & (object_size - 1)) != 0)
                throw Exception(InvalidConfig);

            add_allocator(object_size, offset, allocator_size);

            next_size = object_size * 2;
        }
    }
    else {
        unsigned next_size = MIN_FIXED_ALLOCATOR;
        for (unsigned i = 0; i < DEFAULT_NUM_FIXED_ALLOCATORS; i++) {
            add_allocator(next_size, offset, default_region_size);
            next_size *= 2;
        }
    }

    size_t allocator_region_size = offset;

    init_region_info(allocator_info, "allocator.jdb", addr,
         allocator_region_size);
}

void GraphConfig::init_region_info(RegionInfo &info, const char *name,
                                   uint64_t &addr, size_t size)
{
    size_t alignment = os::get_alignment(size);

    strncpy(info.name, name, RegionInfo::REGION_NAME_LEN);
    info.addr = align(addr, alignment);
    info.len = align(size, alignment);

    addr = info.addr + info.len;
}

void GraphConfig::add_allocator(unsigned object_size, uint64_t &offset, size_t size)
{
    uint64_t pool_offset = align(offset, object_size);
    uint64_t pool_size = align(size, object_size);

    fixed_allocator_info.push_back(
            AllocatorInfo{ object_size, pool_offset, pool_size });

    offset = pool_offset + pool_size;
}
