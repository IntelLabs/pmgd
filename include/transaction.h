#pragma once

#include "graph.h"

namespace Jarvis {
    class TransactionImpl;

    class Transaction {
        TransactionImpl *impl;

        void *operator new(size_t);

    public:
        enum TransactionOptions { Dependent = 0, Independent = 1,
                                  ReadOnly = 0, ReadWrite = 2 };

        Transaction(Graph &db, int options = Dependent|ReadOnly);
        void commit();
        ~Transaction();
    };
};
