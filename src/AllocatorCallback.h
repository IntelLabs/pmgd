#pragma once

#include <list>
#include "TransactionImpl.h"

namespace Jarvis {
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
