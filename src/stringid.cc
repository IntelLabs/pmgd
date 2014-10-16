#include <string>
#include "stringid.h"
#include "exception.h"
#include "StringTable.h"
#include "TransactionImpl.h"
#include "GraphImpl.h"

Jarvis::StringID::StringID(const char *s)
{
    if (s == NULL || *s == '\0')
        _id = 0;
    else {
        TransactionImpl *tx = TransactionImpl::get_tx();
        _id = tx->get_db()->string_table().get(s);
    }
}

std::string Jarvis::StringID::name() const
{
    TransactionImpl *tx = TransactionImpl::get_tx();
    return tx->get_db()->string_table().get(_id);
}
