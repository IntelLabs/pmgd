/**
 * @file   stringid.cc
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

#include <string>
#include "stringid.h"
#include "exception.h"
#include "StringTable.h"
#include "TransactionImpl.h"
#include "GraphImpl.h"

bool PMGD::StringID::get(const char *s, StringID &stringid, bool add)
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

std::string PMGD::StringID::name() const
{
    TransactionImpl *tx = TransactionImpl::get_tx();
    return tx->get_db()->string_table().get(_id);
}
