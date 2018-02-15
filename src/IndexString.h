/**
 * @file   IndexString.h
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
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <locale>

namespace PMGD {
    // String representation for Index nodes
    // This version of the class will always create a PM copy.
    class IndexString {
        class TransientIndexString;

        void operator=(const TransientIndexString &istr);
    protected:
        static const unsigned PREFIX_LEN = 8;

        char _prefix[PREFIX_LEN];
        char *_remainder;
        uint32_t _len;

        IndexString() : _remainder(NULL) { }

    public:
        // Use this constructor to create a PM copy.
        IndexString(const IndexString &istr);

        // The tree code currently does an assignment at delete time.
        // Ideally long strings should be reference counted. But this solution
        // destroys anything at the original and creates a full copy.
        // TODO: Change to reference counting of strings based on overhead
        // analysis or modify tree code to move pointers instead of content.
        void operator=(const IndexString &istr);

        ~IndexString();

        // Function returns 0 for equal, <0 if *this < istr and >0 if *this > istr.
        int compare(const IndexString &) const;

        bool operator==(const IndexString &istr) const
            { return (compare(istr) == 0); }

        bool operator<(const IndexString &istr) const
            { return (compare(istr) < 0); }

        bool operator>(const IndexString &istr) const
            { return (compare(istr) > 0); }

        size_t get_remainder_size()
            { return (_len > PREFIX_LEN)? _len - PREFIX_LEN : 0; }
    };

    // We need to create this class to know which version of _remainder
    // lives in PM vs. DRAM so that we do not try to free something that
    // doesnt belong to our allocators.
    // **This version should be used for all non PM instances of IndexString.
    class TransientIndexString : public IndexString {
        TransientIndexString();

        // Avoid accidental assignments and copies.
        TransientIndexString(const TransientIndexString&);
        TransientIndexString& operator=(const TransientIndexString&);
    public:
        // Doesn't create a copy in PM. But the locale based transformation
        // happens here and gets copied at a DRAM location.
        TransientIndexString(const std::string &str, const std::locale &loc);

        ~TransientIndexString();
    };
}
