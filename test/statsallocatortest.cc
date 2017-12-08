/**
 * @file   statsallocatortest.cc
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
 * This test checks PMGD Allocator Stats
 */

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "pmgd.h"
#include "util.h"
#include <iostream>

using namespace PMGD;

void printStats(std::vector<Graph::AllocatorStats> st)
{
    for (int i = 0; i < st.size(); ++i)
    {
        printf("%s: %lld, %lld, %lld, %lld, %d, %d \n",
        st.at(i).name.c_str(),
        st.at(i).object_size,
        st.at(i).num_objects,
        st.at(i).total_allocated_bytes,
        st.at(i).region_size,
        st.at(i).occupancy,
        st.at(i).health_factor
        );
    }
}

int main(int argc, char **argv)
{
    bool flag_error = false;

    try {

        Graph::Config config;
        config.default_region_size = 8*1024*1024;

        Graph db("statsallocatorgraph", Graph::Create, &config);

        std::vector<Graph::AllocatorStats> st;

        Transaction tx_index(db, Transaction::ReadWrite);

        db.create_index(Graph::NodeIndex, "tag1", "id1", PropertyType::Integer);
        db.create_index(Graph::NodeIndex, "tag1", "id2", PropertyType::Float);
        db.create_index(Graph::NodeIndex, "tag2", "id1", PropertyType::Float);
        db.create_index(Graph::NodeIndex, "tag2", "id2", PropertyType::String);
        db.create_index(Graph::NodeIndex, "tag3", "id1", PropertyType::Integer);
        db.create_index(Graph::EdgeIndex, "etag1", "id1", PropertyType::Integer);
        db.create_index(Graph::NodeIndex, 0, "id3", PropertyType::Integer);

        printf("\nAllocators before allocations: \n");
        st = db.get_allocator_stats();
        printStats(st);

        tx_index.commit();

        for (int i = 0; i < 1000; i++) {
            Transaction tx(db, Transaction::ReadWrite);
            Node &n = db.add_node("tag1");
            n.set_property("id1", i);
            n.set_property("id2", float(i) + 14.23f); // Because we love 14.23
            n.set_property("id3", i+22);
            tx.commit();
        }
        for (int i = 0; i < 2000; i++) {
            Transaction tx(db, Transaction::ReadWrite);
            Node &n = db.add_node("tag2");
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
        for (int i = 0; i < 3000; i++) {
            Transaction tx(db, Transaction::ReadWrite);
            Node &n = db.add_node("tag3");
            if (i % 4 == 0)
            {
                n.set_property("id1", 22 + i);
            }
            else{
                n.set_property("id1", i+33);
            }
            tx.commit();
        }

        Transaction tx3(db, Transaction::ReadWrite);

        printf("\nAllocators after allocations: \n");
        st = db.get_allocator_stats();
        printStats(st);

        if (st[0].num_objects != 6000)
        {
            printf("Number of nodes incorrect\n");
            flag_error = true;
        }
        if (st[0].object_size != 64)
        {
            printf("Node size incorrect\n");
            flag_error = true;
        }
        if (st[1].object_size != 32)
        {
            printf("Edge size incorrect\n");
            flag_error = true;
        }
        /*
         * It will be good to recompute the new data structure sizes
         * and make sure this test can be uncommented.
        if (st[2].total_allocated_bytes != 2856768)
        {
            printf("Allocator size incorrect\n");
            flag_error = true;
        }
        */
        if (st[2].occupancy != 50)
        {
            printf("Allocator occupancy incorrect\n");
            flag_error = true;
        }
        tx3.commit();

        for (int i = 0; i < 6; ++i)
        {
            int counter = 500;
            Transaction tx_remove_half(db, Transaction::ReadWrite);
            for (NodeIterator node_it = db.get_nodes("tag3"); node_it; node_it.next()) {
                db.remove(*node_it);
                if (counter == 0) break;
                --counter;
            }
            tx_remove_half.commit();
        }

        Transaction tx_stats_half(db, Transaction::ReadWrite);

        printf("\nAllocators after half nodes removal: \n");
        st = db.get_allocator_stats();
        printStats(st);
        if (st[0].health_factor != 50)
        {
            printf("Allocator health after half deleted incorrect\n");
            flag_error = true;
        }

        tx_stats_half.commit();

        Transaction tx_remove(db, Transaction::ReadWrite);
        NodeIterator node_it = db.get_nodes();
        tx_remove.commit();
        for (; node_it; node_it.next()) {
            Transaction tx(db, Transaction::ReadWrite);
            db.remove(*node_it);
            tx.commit();
        }

        Transaction tx_stats(db, Transaction::ReadWrite);

        printf("\nAllocators after node removal: \n");
        st = db.get_allocator_stats();
        printStats(st);

        tx_stats.commit();
    }
    catch (PMGD::Exception e) {
        print_exception(e);
        return 1;
    }

    if (flag_error)
    {
        printf("Some errors found... \n");
        return -1;
    }
    else{
        printf("All Allocator stats test succeeded :)\n");
        return 0;
    }
}
