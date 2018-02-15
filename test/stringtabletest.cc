/**
 * @file   stringtabletest.cc
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

/**
 *  This test is for the string table
 */
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdint.h>
#include <vector>
#include <set>
#include <chrono>
#include <ctime>
#include "pmgd.h"
#include "util.h"

using namespace std;
using namespace std::chrono;

using namespace PMGD;

int main()
{
    vector<string> strings = {
        "name", "id", "first-name", "last-name",
        "timestamp", "url", "description", "country",
        "address", "location", "street", "city",
        "county", "employee-id", "email", "user",
        "user-id", "message-id", "transaction-date", "company",
        "profession", "position", "title", "home-phone",
        "cell-phone", "work-phone", "gender", "race",
        "state", "keyword", "contains", "created-on",
        "sell-by", "sold-on", "sent-by", "forwarded-to",
        "weight", "produced-by", "member-of", "created-by",
        "sold-by", "related-to", "daughter-of", "son-of",
        "father-of", "mother-of", "spouse-of", "employed-by",
        "father", "mother", "spouse", "daughter",
        "manages", "managed-by", "constains", "date",
        "publisher", "published-by", "booktitle", "population",
        "author", "employer", "medicine", "treatment",
        "university", "rating", "actor", "actress",
        "movie-name", "item", "price", "director",
        "directed-by", "produced-by", "filename", "review",
        "critique", "editor", "advisor", "professor",
        "wrote", "reviewer", "note", "belongs-to",
        "son", "middle-initial", "mother", "father", // repeating last two for match test
        "envoy\xc3\xa9", "re\xc3\xa7u", // For UTF testing
    };

    int r = 0;
    try {
        Graph db("stringtablegraph", Graph::Create);

        Transaction tx(db, Transaction::ReadWrite);
        cout << "Add and read\n";
        for (size_t i = 0; i < strings.size(); ++i) {
            try {
                StringID s(strings[i].c_str());
                cout << "String: " << s.name() << ", id: " << s.id() << "\n";
            }
            catch (Exception e) {
                print_exception(e);
                r = 1;
            }
        }

        try {
            StringID s("transaction-datumtest");
            cout << "String: " << s.name() << ", id: " << s.id() << "\n";
            r = 1;
        }
        catch (Exception e) {
            if (e.num != InvalidID) {
                print_exception(e);
                r = 1;
            }
        }
    }
    catch (Exception e) {
        print_exception(e);
        r = 1;
    }

    return r;
}
