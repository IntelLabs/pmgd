//
// LDBC query 5
//
#include <string>
#include <vector>
#include <map>
#include <unordered_set>
#include <algorithm>
#include "pmgd.h"
#include "util.h"
#include "neighbor.h"
#include "query5.h"
#include "strings.h"

using namespace PMGD;

class Result
{
    static const int MAX_RESULTS = 20;

    Query5Result results;

public:
    Result(int n) { results.reserve(n); }

    Query5Result get_results() { return results; }

    void add(const Node &forum, int count)
    {
        results.push_back(Query5ResultItem{
                (unsigned long long)forum.get_property(strings.id).int_value(),
                count,
                forum.get_property(strings.title).string_value()
            });
    }

    void sort_and_truncate()
    {
        std::sort(results.begin(), results.end());
        if (results.size() > MAX_RESULTS)
            results.resize(MAX_RESULTS);
    }
};


Query5Result query5(Graph &db, unsigned long long id, time_t date)
{
    strings.init();

    PMGD::Time start_time(gmtime(&date), 0, 0);

    Node &start = *db.get_nodes(strings.Person, PropertyPredicate(strings.id, PropertyPredicate::Eq, id));

    std::map<const Node *, int> counts;

    auto p = [start_time, &counts](Node &friend_node) {
        std::unordered_set<const Node *> fora;

        friend_node.get_edges(strings.hasMember)
            .filter([start_time](const EdgeRef &e) {
                return Disposition(e.get_property(strings.joinDate) >= start_time);
            })
            .process([&fora, &counts](const EdgeRef &e) {
                const Node &forum = e.get_source();
                counts.insert(std::pair<const Node *, int>(&forum, 0));
                fora.insert(&forum);
            });

        get_neighbors(friend_node, Incoming, strings.hasCreator, false)
            .process([&fora, &counts](Node &post) {
                if (post.get_tag() == strings.Post) {
                    const Node &forum = post.get_neighbor(Incoming, strings.containerOf);
                    if (fora.find(&forum) != fora.end()) {
                        counts[&forum]++;
                    }
                }
            });
    };

    get_neighborhood(start, 2, strings.knows, true)
        .process(p);

    Result result(counts.size());
    for (const auto &r : counts)
        result.add(*r.first, r.second);

    result.sort_and_truncate();

    return result.get_results();
}

bool Query5ResultItem::operator<(const Query5ResultItem &a) const
{
    if (count == a.count)
        return forum_id < a.forum_id;
    return a.count < count;
}


#ifdef MAIN
#include <stdio.h>
#include "args.h"
#include "timing.h"

static void print(const Query5Result &r);
static void print(const Query5ResultItem &r);

strings_t strings;

int main(int argc, char **argv)
{
    GET_ARGS

    FILE *f = fopen(filename, "r");
    assert(f != NULL);

    char s[40];
    char *r = fgets(s, sizeof s, f);
    assert(r != NULL);
    assert(strcmp(s, "Person|Date0\n") == 0);

    try {
        Graph db(graph_name, Graph::ReadOnly);

        START_TIMING

        char s[40];
        while (fgets(s, sizeof s, f) != NULL) {
            Transaction tx(db);
            char *e;
            unsigned long long id = strtoull(s, &e, 10);
            assert(*e == '|');
            time_t date = strtoull(e + 1, &e, 10) / 1000; // convert ms to s
            assert(*e == '\n');
            if (verbose) {
                char date_str[40];
                strftime(date_str, sizeof date_str,
                         "%a %b %d %H:%M:%S %Y", gmtime(&date));
                printf("%llu %s\n", id, date_str);
            }

            Query5Result result = query5(db, id, date);

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

static void print(const Query5Result &results)
{
    if (results.empty())
        printf("  no matches\n");
    else
        for (const auto &r : results)
            print(r);
    printf("\n");
}

static void print(const Query5ResultItem &r)
{
    printf("  %s %d\n", r.forum_name.c_str(), r.count);
}
#endif
