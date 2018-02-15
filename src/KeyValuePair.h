/**
 * @file   KeyValuePair.h
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

namespace PMGD {
    template <typename KeyType, typename ValueType> class KeyValuePair {
        KeyType _key;
        ValueType _value;

    public:
        KeyValuePair(const KeyType& k, const ValueType& v)
        {
            _key = k;
            _value = v;
        }
        bool operator==(const KeyValuePair& val2) const
        {
            return (_key == val2._key);
        }
        bool operator<(const KeyValuePair& val2) const
        {
            return (_key < val2._key);
        }
        void set(const KeyType &k, const ValueType &v)
        {
            _key = k;
            _value = v;
        }
        void set_key(const KeyType &k) { _key = k; }
        void set_value(const ValueType &v) { _value = v; }
        const KeyType &key() const { return _key; }
        ValueType &value() { return _value; }
        const ValueType &value() const { return _value; }
    };
}
