/**
 * @file   indexstringtest.cc
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

#include <iostream>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <locale>
#include "pmgd.h"
#include "../src/IndexString.h"
#include "util.h"

using namespace std;
using namespace PMGD;

int main()
{
    try {
        Graph db("indexstringgraph", Graph::Create);
        Transaction tx(db);
        locale loc(std::locale("fr_FR.utf8"));

        cout << "Unit test for index string data type\n";

        TransientIndexString key("Testing", loc);

        TransientIndexString name1("Testing", loc);
        cout << key.compare(name1) << "\n";
        TransientIndexString name2("VTesting", loc);
        cout << key.compare(name2) << "\n";
        TransientIndexString name3("testing", loc);
        cout << key.compare(name3) << "\n";
        TransientIndexString name4("STesting", loc);
        cout << key.compare(name4) << "\n";

        // Long string
        cout << "Testing long strings now\n";
        TransientIndexString lkey("Today is a beautiful day for hiking outside.", loc);

        TransientIndexString lname1("Today is a beautiful day for hiking outside.", loc);
        cout << lkey.compare(lname1) << "\n";

        TransientIndexString lname2("Today is a wonderful day for hiking outside.", loc);
        cout << lkey.compare(lname2) << "\n";

        TransientIndexString lname3("Today IS a wonderful day for hiking outside.", loc);
        cout << lkey.compare(lname3) << "\n";

        TransientIndexString lname4("Today is", loc);
        cout << lkey.compare(lname4) << "\n";

        TransientIndexString lname5("Today i", loc);
        cout << lkey.compare(lname5) << "\n";

        TransientIndexString lname6("Today is a beautiful day for hiking outside. after a long time", loc);
        cout << lkey.compare(lname6) << "\n";

        // French type characters
        cout << "Testing French type characters\n";
        TransientIndexString fkey("envoy\xc3\xa9", loc);

        TransientIndexString fname1("envoy\xc3\xa9", loc);
        cout << fkey.compare(fname1) << "\n";

        TransientIndexString fname2("envoye", loc);
        cout << fkey.compare(fname2) << "\n";

        TransientIndexString fname3("envoyf", loc);
        cout << fkey.compare(fname3) << "\n";

        tx.commit();
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }
    return 0;
}
