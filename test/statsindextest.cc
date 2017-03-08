/**
 * @file   statsindextest.cc
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

/*
 * This test checks PMGD Index Stats
 */

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "pmgd.h"
#include "util.h"
#include <stdlib.h>     /* system, NULL, EXIT_FAILURE */

using namespace PMGD;

void printStats(Graph::IndexStats stats)
{
    printf("%ld, %ld, %ld, %ld, %ld -> Total size: %.2f MB\n",
        stats.unique_entry_size, stats.total_unique_entries,
        stats.total_elements, stats.total_size_bytes,
        stats.health_factor,
        (float)stats.total_size_bytes / 1024.0f / 1024.0f);
}

void printStats(Graph::ChunkStats stats)
{
    printf("%ld, %ld, %ld, %ld, %ld -> Total size: %.2f MB\n",
        stats.total_chunks, stats.chunk_size,
        stats.num_elements, stats.total_size_bytes,
        stats.health_factor,
        (float)stats.total_size_bytes / 1024.0f / 1024.0f);
}

void test_total_bytes(Graph& db, bool& flag_success)
{
    Transaction tx_add_index(db, Transaction::ReadWrite);

    db.create_index(Graph::NodeIndex, "tot_bytes2", "id1", PropertyType::Float);
    db.create_index(Graph::NodeIndex, "tot_bytes2", "id2", PropertyType::String);
    db.create_index(Graph::NodeIndex, "tot_bytes3", "id1", PropertyType::Integer);

    tx_add_index.commit();

    for (int i = 0; i < 100; i++) {
        Transaction tx(db, Transaction::ReadWrite);
        Node &n = db.add_node("tot_bytes2");
        if (i % 2 == 0)
        {
            n.set_property("id2", "string0");
        }
        else{
            n.set_property("id2", "string1");
        }
        n.set_property("id3", i+100);
        tx.commit();
    }
    for (int i = 0; i < 1000; i++) {
        Transaction tx(db, Transaction::ReadWrite);
        Node &n = db.add_node("tot_bytes3");
        if (i % 4 == 0) // Should get a health of 75
        {
            n.set_property("id1", 22);
        }
        else{
            n.set_property("id1", i);
        }
        tx.commit();
    }

    Transaction tx_index(db, Transaction::ReadWrite);

    Graph::IndexStats st;

    printf("Stats tot_bytes2 id2\n");
    st = db.get_index_stats(Graph::NodeIndex, "tot_bytes2", "id2");
    if (st.total_size_bytes != 1744){
        // Total size should be
        // 16 + 64*2 + 100*16 = 1744
        // sizeof(AvlTreeIndex) = 16
        // sizeof(TreeNode) = 64
        // 2 Keys
        // 100 elements, each of 16
        printf("-----------> tot_bytes2: id2 index wrong size\n");
        flag_success = false;
    }
    if (st.total_unique_entries != 2){
        printf("-----------> tot_bytes2: id2 index wrong size\n");
        flag_success = false;
    }
    printStats(st);

    printf("Stats tot_bytes3 id1\n");
    st = db.get_index_stats(Graph::NodeIndex, "tot_bytes3", "id1");
    if (st.total_size_bytes != 52016){
        // Total size should be
        // 16 + 48*750 + 1000*16 = 52016
        // sizeof(AvlTreeIndex) = 16
        // sizeof(TreeNode) = 48
        // 750 Keys
        // 1000 elements, each of 16
        printf("-----------> tot_bytes3: id1 index wrong size\n");
        flag_success = false;
    }
    printStats(st);

    tx_index.commit();
}

void test_string_remainder_case(Graph& db, bool& flag_success)
{
    Transaction tx_string(db, Transaction::ReadWrite);

    db.create_index(Graph::NodeIndex, "string", "id1", PropertyType::String);

    for (int i = 0; i < 2; ++i){
        Node &n = db.add_node("string");
        if (i % 2 == 0)
        {
            n.set_property("id1", "shortstr");
        }
        else{
            // This string is 12 chars larger than
            // the prefix len in IndexString,
            // thus, the whole index should by
            // 196 bytes.
            n.set_property("id1", "justabitlongerstring");
        }
    }

    printf("Stats string id1\n");
    Graph::IndexStats st = db.get_index_stats(Graph::NodeIndex, "string", "id1");
    if (st.total_size_bytes != 188){
        printf("-----------> string: id1 index wrong size\n");
        flag_success = false;
    }
    printStats(st);

    tx_string.commit();
}

void test_empty_index(Graph& db, bool& flag_success)
{
    printf("Testing existing empty test\n");

    Transaction tx_add_idx(db, Transaction::ReadWrite);
    db.create_index(Graph::NodeIndex, "empty", "id1", PropertyType::Integer);
    db.create_index(Graph::NodeIndex, "empty", "id2", PropertyType::Float);
    tx_add_idx.commit();

    Transaction tx_index(db, Transaction::ReadWrite);

    Graph::IndexStats st = db.get_index_stats(Graph::NodeIndex, "empty", "id1");
    if (st.total_size_bytes != 16){
        printf("-----------> empty: id1 index wrong size\n");
        flag_success = false;
    }
    if (st.unique_entry_size != 48){
        printf("-----------> empty: id1 index wrong entry size\n");
        flag_success = false;
    }
    printStats(st);

    tx_index.commit();
}

void test_total_elements(Graph& db, bool& flag_success)
{
    Transaction tx_add_idx(db, Transaction::ReadWrite);
    db.create_index(Graph::NodeIndex, "tot_elem1", "id1", PropertyType::Integer);
    db.create_index(Graph::NodeIndex, "tot_elem1", "id2", PropertyType::Float);
    tx_add_idx.commit();

    for (int i = 0; i < 100; i++) {
        Transaction tx(db, Transaction::ReadWrite);
        Node &n = db.add_node("tot_elem1");
        n.set_property("id1", i);
        n.set_property("id2", float(i) + 14.23f); // Because we love 14.23
        n.set_property("id3", i);
        tx.commit();
    }

    Transaction tx_stats(db, Transaction::ReadWrite);

    Graph::IndexStats st;

    printf("Stats tot_elem1 id1\n");
    st = db.get_index_stats(Graph::NodeIndex, "tot_elem1", "id1");
    if (st.total_elements != 100){
        printf("-----------> tot_elem1: id1 index wrong elements\n");
        flag_success = false;
    }
    printStats(st);

    printf("Stats tot_elem1 id2\n");
    st = db.get_index_stats(Graph::NodeIndex, "tot_elem1", "id2");
    if (st.total_elements != 100){
        printf("-----------> tot_elem1: id2 index wrong elements\n");
        flag_success = false;
    }
    printStats(st);

    tx_stats.commit();
}

void test_health_factor(Graph& db, bool& flag_success)
{
    Transaction tx_add_idx(db, Transaction::ReadWrite);
    db.create_index(Graph::NodeIndex, "health1", "id1", PropertyType::Integer);
    db.create_index(Graph::NodeIndex, "health2", "id1", PropertyType::Integer);
    tx_add_idx.commit();

    for (int i = 0; i < 1000; i++) {
        Transaction tx(db, Transaction::ReadWrite);
        Node &n = db.add_node("health1");
        if (i % 4 == 0) // Should get a health of 75
        {
            n.set_property("id1", 22);
        }
        else{
            n.set_property("id1", i);
        }
        tx.commit();
    }
    for (int i = 0; i < 3; i++) {
        Transaction tx(db, Transaction::ReadWrite);
        Node &n = db.add_node("health2");
        if (i % 3 == 0) // Should get a health of 75
        {
            n.set_property("id1", 22);
        }
        else{
            n.set_property("id1", 33);
        }
        tx.commit();
    }

    Transaction tx_stats(db, Transaction::ReadWrite);

    Graph::IndexStats st;

    printf("Stats health1 id1\n");
    st = db.get_index_stats(Graph::NodeIndex, "health1", "id1");
    if (st.health_factor != 75){
        printf("-----------> health1: id1 Health Factor is wrong\n");
        flag_success = false;
    }
    printStats(st);

    printf("Stats health1 0\n");
    st = db.get_index_stats(Graph::NodeIndex, "health1", 0);
    if (st.health_factor != 100){
        printf("-----------> Tag3 0: Health Factor is wrong\n");
        flag_success = false;
    }
    printStats(st);

    printf("Stats health2 id1\n");
    st = db.get_index_stats(Graph::NodeIndex, "health2", "id1");

    if (st.health_factor != 34){
        printf("-----------> health2: id1 Health Factor is wrong\n");
        flag_success = false;
    }
    printStats(st);

    tx_stats.commit();
}

void test_non_existent_index(Graph& db, bool& flag_success)
{
    Transaction tx_index(db, Transaction::ReadWrite);

    printf("Test NonExistent index\n");
    Graph::IndexStats st = db.get_index_stats(Graph::EdgeIndex, "NonExistent", "id1");
    if (st.total_size_bytes != 0){
        printf("-----------> NonExistent: id1 index wrong size\n");
        flag_success = false;
    }
    printStats(st);

    tx_index.commit();
}

void test_tag_level_index(Graph& db, bool& flag_success)
{
    Transaction tx_add_idx(db, Transaction::ReadWrite);
    db.create_index(Graph::NodeIndex, "test_tag1", "id1", PropertyType::Integer);
    tx_add_idx.commit();

    for (int i = 0; i < 1000; i++) {
        Transaction tx(db, Transaction::ReadWrite);
        Node &n = db.add_node("test_tag1");
        if (i % 4 == 0) // Should get a health of 75
        {
            n.set_property("id1", 22);
        }
        else{
            n.set_property("id1", i);
        }
        tx.commit();
    }

    Transaction tx_index(db, Transaction::ReadWrite);

    printf("Test Tag-Level index\n");
    Graph::IndexStats st = db.get_index_stats(Graph::NodeIndex, "test_tag1");
    if (st.health_factor != 75){
        printf("-----------> test_tag1: Health Factor is wrong\n");
        flag_success = false;
    }
    printStats(st);
    tx_index.commit();
}

void test_type_level_index(Graph& db, bool& flag_success)
{
    Transaction tx_add_idx(db, Transaction::ReadWrite);
    db.create_index(Graph::EdgeIndex, "test_edge1", "id1", PropertyType::Integer);
    tx_add_idx.commit();

    for (int i = 0; i < 1000; i++) {
        Transaction tx(db, Transaction::ReadWrite);
        Node &n1 = db.add_node("test_tag1");
        Node &n2 = db.add_node("test_tag1");
        Edge & e = db.add_edge(n1,n2, "test_edge1");
        e.set_property("id1",2+i);
        tx.commit();
    }

    Transaction tx_index(db, Transaction::ReadWrite);

    printf("Test Type-Level NodeIndex\n");
    Graph::IndexStats st = db.get_index_stats(Graph::NodeIndex);
    // if (st.health_factor != 75){
    //     printf("-----------> test_tag1: Health Factor is wrong\n");
    //     flag_success = false;
    // }
    printStats(st);
    printf("Test Type-Level EdgeIndex\n");
    st = db.get_index_stats(Graph::EdgeIndex);
    if (st.health_factor != 100){
        printf("-----------> EdgeIndex: Health Factor is wrong\n");
        flag_success = false;
    }
    printStats(st);
    tx_index.commit();
}

void test_top_level_index(Graph& db, bool& flag_success)
{
    Transaction tx_index(db, Transaction::ReadWrite);

    printf("Test Top-Level index\n");
    Graph::IndexStats st = db.get_index_stats();
    // (75*8510/10510 + 100*2000/10510) = 78.8
    if (st.health_factor != 78){
        printf("-----------> top-level-idx: Health Factor is wrong\n");
        flag_success = false;
    }
    printStats(st);
    tx_index.commit();
}

void test_tag_level_chunk(Graph& db, bool& flag_success)
{
    Transaction tx(db, Transaction::ReadWrite);
    printf("Test Tag-Level chunk\n");
    Graph::ChunkStats st = db.get_chunk_list_stats(Graph::NodeIndex, "test_tag1");
    // if (st.health_factor != 75){
    //     printf("-----------> test_tag1: Health Factor is wrong\n");
    //     flag_success = false;
    // }
    printStats(st);
    tx.commit();
}

void test_type_level_chunk(Graph& db, bool& flag_success)
{
    Transaction tx(db, Transaction::ReadWrite);
    db.create_index(Graph::NodeIndex, "test_edgee1", "id1", PropertyType::Integer);
    printf("Test Type-Level chunk Index\n");
    Graph::ChunkStats st = db.get_chunk_list_stats(Graph::NodeIndex);
    if (st.health_factor != 75){
        printf("-----------> chunk Index: Health Factor is wrong\n");
        flag_success = false;
    }
    printStats(st);
    printf("Test Type-Level chunk Edge\n");
    st = db.get_chunk_list_stats(Graph::EdgeIndex);
    if (st.health_factor != 25){
        printf("-----------> chunk Edge: Health Factor is wrong\n");
        flag_success = false;
    }
    printStats(st);
    tx.commit();
}

void test_top_level_chunk(Graph& db, bool& flag_success)
{
    Transaction tx(db, Transaction::ReadWrite);
    printf("Test Top-Level chunk\n");
    Graph::ChunkStats st = db.get_all_chunk_lists_stats();
    // if (st.health_factor != 75){
    //     printf("-----------> test_tag1: Health Factor is wrong\n");
    //     flag_success = false;
    // }
    printStats(st);
    tx.commit();
}

int main(int argc, char **argv)
{
    bool flag_success = true;

    try {
        Graph db("statsindexgraph", Graph::Create);

        test_total_elements         (db, flag_success);
        test_health_factor          (db, flag_success);
        test_total_bytes            (db, flag_success);
        test_empty_index            (db, flag_success);
        test_non_existent_index     (db, flag_success);
        test_string_remainder_case  (db, flag_success);
        test_tag_level_index        (db, flag_success);
        test_type_level_index       (db, flag_success);
        test_top_level_index        (db, flag_success);
        test_tag_level_chunk        (db, flag_success);
        test_type_level_chunk       (db, flag_success);
        test_top_level_chunk        (db, flag_success);
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    if (flag_success){
        printf("All test succeeded :)\n");
    }
    else{
        printf("Failed :(\n");
        return -1;
    }

    return 0;
}
