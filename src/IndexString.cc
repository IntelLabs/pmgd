/**
 * @file   IndexString.cc
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

#include <assert.h>
#include "IndexString.h"
#include "GraphImpl.h"
#include "TransactionImpl.h"
#include "Allocator.h"

using namespace PMGD;

IndexString::IndexString(const IndexString &istr)
{
    _len = istr._len;
    memcpy(_prefix, istr._prefix, PREFIX_LEN);

    if (_len > PREFIX_LEN) {
        uint32_t remaining = _len - PREFIX_LEN;
        // Use the Transaction object to get to the allocator
        // so that the tree code can just call the constructor
        // with the key and not have to pass the allocator object.
        // Otherwise placement new will stop working for basic
        // datatypes.
        TransactionImpl *tx = TransactionImpl::get_tx();
        Allocator &allocator = tx->get_db()->allocator();
        _remainder = (char *)allocator.alloc(remaining * sizeof(char));

        // _remainder is the only part of string that is not inlined.
        // So it needs a separate flush.
        tx->write_nolog(_remainder, istr._remainder, remaining);
    }
    else
        _remainder = NULL;
}

void IndexString::operator=(const IndexString &istr)
{
    // The tree code already logs inline items. So no need to
    // log here.
    memcpy(_prefix, istr._prefix, PREFIX_LEN);

    TransactionImpl *tx = TransactionImpl::get_tx();
    Allocator &allocator = tx->get_db()->allocator();
    if (_remainder) {
        if (_len == istr._len) {
            uint32_t remaining = _len - PREFIX_LEN;
            tx->write(_remainder, istr._remainder, remaining);
            return;
        }
        else
            allocator.free(_remainder, _len - PREFIX_LEN);
    }

    // The original was short string but new one might be longer.
    _len = istr._len;
    if (_len > PREFIX_LEN) {
        uint32_t remaining = _len - PREFIX_LEN;
        _remainder = (char *)allocator.alloc(remaining * sizeof(char));
        tx->write_nolog(_remainder, istr._remainder, remaining);
    }
    else
        _remainder = NULL;
}

IndexString::~IndexString()
{
    if (_remainder) {
        TransactionImpl *tx = TransactionImpl::get_tx();
        Allocator &allocator = tx->get_db()->allocator();
        allocator.free(_remainder, _len - PREFIX_LEN);
    }
}

int IndexString::compare(const IndexString &istr) const
{
    int ret_val = memcmp(_prefix, istr._prefix, PREFIX_LEN);
    if (ret_val != 0)
        return ret_val;
    // Else go for the long one.
    // But make sure to account for one long and one short string
    uint32_t shorter = (_len > istr._len) ? istr._len : _len;
    if (shorter > PREFIX_LEN) {
        uint32_t len = shorter - PREFIX_LEN;
        ret_val = memcmp(_remainder, istr._remainder, len);
        if (ret_val != 0)
            return ret_val;
    }

    return (long long)_len - (long long)istr._len;
}

TransientIndexString::TransientIndexString(const std::string &str,
                                            const std::locale &loc)
{
    // Transform and store so that comparisons become faster
    // This also makes it easier to break the string at any point
    // without losing multi-byte characters.
    const std::collate<char>& col = std::use_facet<std::collate<char> >(loc);
    std::string tstr = col.transform(str.data(), str.data() + str.length());
    _len = tstr.length();

    if (_len > PREFIX_LEN) {
        memcpy(_prefix, tstr.data(), PREFIX_LEN);
        uint32_t remaining = _len - PREFIX_LEN;
        _remainder = (char *)malloc(remaining * sizeof(char));
        memcpy(_remainder, tstr.data() + PREFIX_LEN, remaining);
    }
    else {
        memcpy(_prefix, tstr.data(), _len);
        memset(_prefix + _len, 0, PREFIX_LEN - _len);
    }
}

TransientIndexString::~TransientIndexString()
{
    // This destructor will be followed by its parent's destructor which
    // tries to free _remainder to our allocator if it is non NULL.
    if (_remainder) {
        free(_remainder);
        _remainder = NULL;
    }
}
