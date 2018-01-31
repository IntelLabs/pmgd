/**
 * @file   AllocatorCallback.h
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

#include <list>
#include "TransactionImpl.h"

namespace PMGD {
    template <typename A, typename S>
    class AllocatorCallback
    {
        A *_allocator;
        std::list<S> _list;
    public:
        AllocatorCallback(A *a) : _allocator(a) { }

        void operator()(TransactionImpl *tx)
            { _allocator->clean_free_list(tx, _list); }

        void add(S s) { _list.push_front(s); }

        static void delayed_free(TransactionImpl *tx, A *allocator, S s)
        {
            auto *f = tx->lookup_callback(allocator);
            if (f == NULL) {
                tx->register_commit_callback(allocator, AllocatorCallback(allocator));

                // The callback object is copied when it is registered,
                // so we have to call lookup again to get a pointer to
                // the stored object.
                f = tx->lookup_callback(allocator);
            }

            auto *cb = f->template target<AllocatorCallback>();
            cb->add(s);
        }
    };
}
