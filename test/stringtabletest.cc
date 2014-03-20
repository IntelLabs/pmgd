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
#include "jarvis.h"
#include "../util/util.h"

using namespace std;
using namespace std::chrono;

using namespace Jarvis;

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
        "transaction-datumtest", "envoy\xc3\xa9", "re\xc3\xa7u", // For > 16, UTF testing
    };

    try {
        Graph db("stringtablegraph", Graph::Create);

        Transaction tx(db);
        cout << "Add and read\n";
        for (size_t i = 0; i < strings.size(); ++i) {
            try {
                StringID s(strings[i].c_str());
                cout << "String: " << s.name() << ", id: " << s.id() << "\n";
            }
            catch (Exception e) {
                print_exception(e);
            }
        }
    }
    catch (Exception e) {
        print_exception(e);
    }

    return 0;
}
