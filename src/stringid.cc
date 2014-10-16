#include <string>
#include "stringid.h"
#include "exception.h"
#include "StringTable.h"
#include "TransactionImpl.h"
#include "GraphImpl.h"

bool Jarvis::StringID::get(const char *s, StringID &stringid, bool add)
{
    if (s == NULL || *s == '\0') {
        stringid._id = 0;
        return true;
    }
    else {
        TransactionImpl *tx = TransactionImpl::get_tx();
        return tx->get_db()->string_table().get(s, stringid._id, add);
    }
}

std::string Jarvis::StringID::name() const
{
    TransactionImpl *tx = TransactionImpl::get_tx();
    return tx->get_db()->string_table().get(_id);
}
