//
// LDBC query 6
//
#include <string>
#include <vector>
#include <map>
#include <unordered_set>
#include <algorithm>
#include "pmgd.h"
#include "util.h"
#include "neighbor.h"
#include "query6.h"
#include "strings.h"

using namespace PMGD;

class Result
{
    static const int MAX_RESULTS = 10;

    Query6Result results;

public:
    Result(int n) { results.reserve(n); }

    Query6Result get_results() { return results; }

    void add(std::string name, int count)
        { results.push_back(Query6ResultItem{ name, count }); }

    void sort_and_truncate()
    {
        std::sort(results.begin(), results.end());
        if (results.size() > MAX_RESULTS)
            results.resize(MAX_RESULTS);
    }
};


Query6Result query6(Graph &db, unsigned long long id, const char *tag_name)
{
    strings.init();

    const Node &start = *db.get_nodes(strings.Person, PropertyPredicate(strings.id, PropertyPredicate::Eq, id));
    const Node &tag = *db.get_nodes(strings.Tag, PropertyPredicate(strings.name, PropertyPredicate::Eq, tag_name));

    std::map<const Node *, int> counts;

    auto p = [&tag, &counts](Node &friend_node) {
        get_neighbors(friend_node, Incoming, strings.hasCreator, false)
            .filter([](const Node &post) {
                return Disposition(post.get_tag() == strings.Post);
            })
            .filter([&tag](const Node &post) {
                return Disposition(bool(
                    get_neighbors(post, Outgoing, strings.hasTag, false)
                        .filter([&tag](const Node &t) {
                            return Disposition(t == tag);
                        })));
            })
            .process([&tag, &counts](const Node &post) {
                get_neighbors(post, Outgoing, strings.hasTag, false)
                    .process([&tag, &counts](const Node &t) {
                        if (t != tag) {
                            auto iter = counts.find(&t);
                            if (iter == counts.end())
                                counts[&t] = 1;
                            else
                                iter->second++;
                        }
                    });
            });
    };

    get_neighborhood(start, 2, strings.knows, true)
        .process(p);


    Result result(counts.size());
    for (const auto &i : counts) {
        std::string name = i.first->get_property(strings.name).string_value();
        result.add(name, i.second);
    }

    result.sort_and_truncate();

    return result.get_results();
}

bool Query6ResultItem::operator<(const Query6ResultItem &a) const
{
    return count == a.count ? name < a.name : count > a.count;
}


#ifdef MAIN
#include <stdio.h>
#include "args.h"
#include "timing.h"

static void print(const Query6Result &);

strings_t strings;

int main(int argc, char **argv)
{
    GET_ARGS

    FILE *f = fopen(filename, "r");
    assert(f != NULL);

    char s[120];
    char *r = fgets(s, sizeof s, f);
    assert(r != NULL);
    assert(strcmp(s, "Person|Tag\n") == 0);

    try {
        Graph db(graph_name, Graph::ReadOnly);

        START_TIMING

        while (fgets(s, sizeof s, f) != NULL) {
            Transaction tx(db);
            char *e;
            unsigned long long id = strtoull(s, &e, 10);
            assert(*e == '|');
            const char *tag = e + 1;
            e = strchr(e + 1, '\n');
            assert(e != NULL);
            *e = '\0';
            if (verbose) printf("%llu %s\n", id, tag);

            Query6Result result = query6(db, id, tag);

            if (verbose) print(result);
        }

        END_TIMING
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}

static void print(const Query6Result &results)
{
    if (results.empty())
        printf("  no matches\n");
    else
        for (const auto &r : results)
            printf("  %s %d\n", r.name.c_str(), r.count);
    printf("\n");
}
#endif
