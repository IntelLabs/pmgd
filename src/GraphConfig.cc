#include <stddef.h>
#include <string.h>
#include <locale.h>
#include "graph.h"
#include "GraphConfig.h"
#include "GraphImpl.h"
#include "allocator.h"

using namespace Jarvis;

static const size_t DEFAULT_REGION_SIZE = 0x10000000000;
static const size_t DEFAULT_NODE_SIZE = 64;
static const size_t DEFAULT_EDGE_SIZE = 32;
static const size_t DEFAULT_NUM_FIXED_ALLOCATORS = 5;
static const size_t MIN_FIXED_ALLOCATOR = 16;

static const size_t INDEX_MANAGER_SIZE = 4096;

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
        throw Exception(invalid_config);
}

GraphConfig::GraphConfig(const Graph::Config *user_config)
{
#define VALUE(field, default) \
            (user_config != NULL && user_config->field != 0 \
                ? user_config->field : (default))

    size_t default_region_size = VALUE(default_region_size,DEFAULT_REGION_SIZE);

    node_size = VALUE(node_size, DEFAULT_NODE_SIZE);
    if (node_size < DEFAULT_NODE_SIZE)
        throw Exception(invalid_config);
    check_power_of_two(node_size);

    edge_size = VALUE(edge_size, DEFAULT_EDGE_SIZE);
    if (edge_size < DEFAULT_EDGE_SIZE)
        throw Exception(invalid_config);
    check_power_of_two(edge_size);

    max_stringid_length = VALUE(max_stringid_length, DEFAULT_MAX_STRINGID_LENGTH);
    check_power_of_two(max_stringid_length);

    locale_name = user_config != NULL && !user_config->locale_name.empty()
                      ? user_config->locale_name : std::locale().name();

    size_t node_table_size = VALUE(node_table_size, default_region_size);
    if (node_table_size % node_size != 0)
        throw Exception(invalid_config);

    size_t edge_table_size = VALUE(edge_table_size, default_region_size);
    if (edge_table_size % edge_size != 0)
        throw Exception(invalid_config);

    size_t string_table_size = VALUE(string_table_size, DEFAULT_STRING_TABLE_SIZE);
    check_power_of_two(string_table_size);

    uint64_t addr = BASE_ADDRESS + INFO_SIZE;
    init_region_info(indexmanager_info, "indexmanager.jdb", addr,
         INDEX_MANAGER_SIZE);
    init_region_info(stringtable_info, "stringtable.jdb", addr,
        string_table_size);
    init_region_info(transaction_info, "transaction.jdb", addr,
         TRANSACTION_REGION_SIZE);
    init_region_info(node_info, "nodes.jdb", addr, node_table_size);
    init_region_info(edge_info, "edges.jdb", addr, edge_table_size);

    size_t offset = 0;

    if (user_config != NULL && user_config->fixed_allocators.size() != 0) {
        if (user_config->fixed_allocators.size() > GraphImpl::MAX_FIXED_ALLOCATORS)
            throw Exception(invalid_config);

        unsigned next_size = MIN_FIXED_ALLOCATOR;
        for (auto a : user_config->fixed_allocators) {
            unsigned object_size = a.object_size;
            size_t allocator_size = a.size;

            if (object_size < next_size
                    || (object_size & (object_size - 1)) != 0
                    || (allocator_size & (object_size - 1)) != 0)
                throw Exception(invalid_config);

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
    size_t alignment;
    if (size >= 0x40000000)
        alignment = 0x40000000;
    else if (size >= 0x200000)
        alignment = 0x200000;
    else
        alignment = 0x1000;

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
