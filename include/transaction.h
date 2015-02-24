#pragma once

#include <stddef.h>
#include "graph.h"

namespace Jarvis {
    class TransactionImpl;

    class Transaction {
        TransactionImpl *_impl;

    public:
        enum TransactionOptions { Dependent = 0, Independent = 1,
                                  ReadOnly = 0, ReadWrite = 2 };

        Transaction(const Transaction &) = delete;
        void operator=(const Transaction &) = delete;
        Transaction(Graph &db, int options = Dependent|ReadOnly);
        void commit();
        ~Transaction();
    };
};
