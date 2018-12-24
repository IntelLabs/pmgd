/**
 * @file   AvlTreeIndex.h
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
#include "Index.h"
#include "AvlTree.h"
#include "TransactionImpl.h"

namespace PMGD {
    template<typename K, typename V> class AvlTreeIndex
                            : public Index, public AvlTree<K,V>
    {
        typedef typename AvlTree<K,V>::TreeNode TreeNode;

        // Helper functions for the iterators to function.
        class Compare;
        class Stack;

        void find_start(TreeNode *root, const Compare &cmin, const Compare &cmax,
                        Stack &path, TransactionImpl *tx);
        void add_right_tree(TreeNode *root, const Compare &cmax,
                            Stack &path, TransactionImpl *tx);
        void find_start_min(TreeNode *root, const Compare &cmax,
                            Stack &path, TransactionImpl *tx);
        void find_start_max(TreeNode *root, const Compare &cmin,
                            Stack &path, TransactionImpl *tx);
        void add_full_right_tree(TreeNode *root, Stack &path, TransactionImpl *tx);
        void find_start_all(TreeNode *root, Stack &path, TransactionImpl *tx);
        void add_nodes_neq(TreeNode *root, const K &neq, Stack &path, TransactionImpl *tx);
        void find_node_neq(TreeNode *root, const Compare &cur,
                           const K &neq, Stack &path, TransactionImpl *tx);

        // For reverse iterators
        void find_start_reverse(TreeNode *root, const Compare &cmin, const Compare &cmax,
                        Stack &path, TransactionImpl *tx);
        void add_left_tree(TreeNode *root, const Compare &cmin,
                            Stack &path, TransactionImpl *tx);
        void find_start_max_reverse(TreeNode *root, const Compare &cmin,
                            Stack &path, TransactionImpl *tx);
        void find_start_min_reverse(TreeNode *root, const Compare &cmax,
                            Stack &path, TransactionImpl *tx);
        void find_start_all_reverse(TreeNode *root, Stack &path, TransactionImpl *tx);
        void add_full_left_tree(TreeNode *root, Stack &path, TransactionImpl *tx);
        void add_nodes_neq_reverse(TreeNode *root, const K &neq, Stack &path, TransactionImpl *tx);
        void find_node_neq_reverse(TreeNode *root, const Compare &cur,
                                   const K &neq, Stack &path, TransactionImpl *tx);

        // For statistics
        void stats_recursive(TreeNode *root, Graph::IndexStats &stats);
        void stats_health_recursive(TreeNode *root, Graph::IndexStats &stats, size_t &avg_elem_per_node);

        template <class D> friend class Index_IteratorImplBase;
        template <class D> friend class IndexEq_IteratorImpl;
        template <class D> friend class IndexRange_IteratorImpl;
        template <class D> friend class IndexRangeNomax_IteratorImpl;
        template <class D> friend class IndexRangeNeq_IteratorImpl;
        template <class D> friend class IndexRangeReverse_IteratorImpl;
        template <class D> friend class IndexRangeNomin_IteratorImpl;
        template <class D> friend class IndexRangeNeqReverse_IteratorImpl;

    public:
        // Initialize both and they do their own transaction flush
        AvlTreeIndex(PropertyType ptype) : Index(ptype), AvlTree<K,V>()
        {
            // This will flush for both the base classes too.
            TransactionImpl *tx = TransactionImpl::get_tx();
            tx->flush_range(this, sizeof *this);
        }

        using AvlTree<K,V>::add;
        using AvlTree<K,V>::remove;

        Index::Index_IteratorImplIntf *get_iterator(Graph::IndexType index_type, bool reverse);
        Index::Index_IteratorImplIntf *get_iterator(Graph::IndexType index_type, const K &key,
                                                    PropertyPredicate::Op op, bool reverse);
        Index::Index_IteratorImplIntf *get_iterator(Graph::IndexType index_type, const K &min,
                                                    const K &max, PropertyPredicate::Op op,
                                                    bool reverse);

        // For statistics
        void index_stats_info(Graph::IndexStats &stats);
    };

    // For the actual property value indices
    class IndexString;
    template <class T> class List;
    typedef AvlTreeIndex<long long, List<void *>> LongValueIndex;
    typedef AvlTreeIndex<double, List<void *>> FloatValueIndex;
    typedef AvlTreeIndex<bool, List<void *>> BoolValueIndex;
    typedef AvlTreeIndex<Time, List<void *>> TimeValueIndex;
    typedef AvlTreeIndex<IndexString, List<void *>> StringValueIndex;
}
