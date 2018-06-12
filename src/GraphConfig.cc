/**
 * @file   GraphConfig.cc
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

#include <stddef.h>
#include <string.h>
#include <locale.h>
#include <thread>
#include "graph.h"
#include "GraphConfig.h"
#include "GraphImpl.h"
#include "Allocator.h"

using namespace PMGD;

static const size_t DEFAULT_NODE_SIZE = 64;
static const size_t DEFAULT_EDGE_SIZE = 32;

static const size_t DEFAULT_TRANSACTION_TABLE_SIZE = SIZE_4KB;
static const size_t DEFAULT_JOURNAL_SIZE = 64 * SIZE_2MB;

static const size_t INDEX_MANAGER_SIZE = SIZE_4KB;

static const int DEFAULT_MAX_STRINGID_LENGTH = 16;
static const int DEFAULT_MAX_STRINGIDS = 4096;
static const size_t DEFAULT_STRING_TABLE_SIZE = DEFAULT_MAX_STRINGIDS * DEFAULT_MAX_STRINGID_LENGTH;

static const unsigned DEFAULT_NUM_ALLOCATORS = 1;

static const size_t DEFAULT_STRIPED_LOCK_SIZE = SIZE_2MB;
static const unsigned DEFAULT_STRIPE_WIDTH = 64;  // bytes

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
        throw PMGDException(InvalidConfig);
}

GraphConfig::GraphConfig(const Graph::Config *user_config)
{
#define VALUE(field, default) \
            (user_config != NULL && user_config->field != 0 \
                ? user_config->field : (default))

    size_t default_region_size = VALUE(default_region_size,os::get_default_region_size());

    node_size = VALUE(node_size, DEFAULT_NODE_SIZE);
    if (node_size < DEFAULT_NODE_SIZE)
        throw PMGDException(InvalidConfig);
    check_power_of_two(node_size);

    edge_size = VALUE(edge_size, DEFAULT_EDGE_SIZE);
    if (edge_size < DEFAULT_EDGE_SIZE)
        throw PMGDException(InvalidConfig);
    check_power_of_two(edge_size);

    max_stringid_length = VALUE(max_stringid_length, DEFAULT_MAX_STRINGID_LENGTH);
    check_power_of_two(max_stringid_length);

    size_t transaction_table_size = VALUE(transaction_table_size, DEFAULT_TRANSACTION_TABLE_SIZE);
    size_t journal_size = VALUE(journal_size, DEFAULT_JOURNAL_SIZE);

    locale_name = user_config != NULL && !user_config->locale_name.empty()
                      ? user_config->locale_name : std::locale().name();

    size_t node_table_size = VALUE(node_table_size, default_region_size);
    if (node_table_size % node_size != 0)
        throw PMGDException(InvalidConfig);

    size_t edge_table_size = VALUE(edge_table_size, default_region_size);
    if (edge_table_size % edge_size != 0)
        throw PMGDException(InvalidConfig);

    size_t string_table_size = VALUE(string_table_size, DEFAULT_STRING_TABLE_SIZE);
    check_power_of_two(string_table_size);

    size_t allocator_region_size = VALUE(allocator_region_size, default_region_size);
    if (allocator_region_size % Allocator::CHUNK_SIZE != 0)
        throw PMGDException(InvalidConfig, "Invalid allocator region size");

    // Put constraints on number of instances based on the region size
    // and core count.
    num_allocators = VALUE(num_allocators, DEFAULT_NUM_ALLOCATORS);
    if (allocator_region_size < 2 * Allocator::CHUNK_SIZE || num_allocators < 1)
        throw PMGDException(InvalidConfig, "Cannot even support one allocator instance");
    if (num_allocators * 2 * Allocator::CHUNK_SIZE > allocator_region_size)
        throw PMGDException(InvalidConfig, "Not enough space to create so many allocators\n");
    if (num_allocators > std::thread::hardware_concurrency())
        throw PMGDException(InvalidConfig, "Max allocators allowed: " +
                                       std::to_string(std::thread::hardware_concurrency()));

    size_t default_striped_lock_size;
    default_striped_lock_size = VALUE(default_striped_lock_size, DEFAULT_STRIPED_LOCK_SIZE);
    check_power_of_two(default_striped_lock_size);
    node_striped_lock_size = VALUE(node_striped_lock_size, default_striped_lock_size);
    check_power_of_two(node_striped_lock_size);
    edge_striped_lock_size = VALUE(edge_striped_lock_size, default_striped_lock_size);
    check_power_of_two(edge_striped_lock_size);
    index_striped_lock_size = VALUE(index_striped_lock_size, default_striped_lock_size);
    check_power_of_two(index_striped_lock_size);

    unsigned default_width = VALUE(default_stripe_width, DEFAULT_STRIPE_WIDTH);
    node_stripe_width = VALUE(node_stripe_width, default_width);
    edge_stripe_width = VALUE(edge_stripe_width, default_width);
    index_stripe_width = VALUE(index_stripe_width, default_width);

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
