//
// LDBC query 4
//
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include "pmgd.h"
#include "util.h"
#include "neighbor.h"
#include "query4.h"
#include "strings.h"

using namespace PMGD;

class Result
{
    static const int MAX_RESULTS = 10;

    Query4Result results;

public:
    Result(int n) { results.reserve(n); }

    Query4Result get_results() { return results; }

    void add(std::string name, int count)
        { results.push_back(Query4ResultItem{ name, count }); }

    void sort_and_truncate()
    {
        std::sort(results.begin(), results.end());
        if (results.size() > MAX_RESULTS)
            results.resize(MAX_RESULTS);
    }
};


Query4Result query4(Graph &db, unsigned long long id,
                           time_t start_date, time_t end_date)
{
    strings.init();

    PMGD::Time start_time(gmtime(&start_date), 0, 0);
    PMGD::Time end_time(gmtime(&end_date), 0, 0);

    Node &start = *db.get_nodes(strings.Person, PropertyPredicate(strings.id, PropertyPredicate::Eq, id));

    std::map<const Node *, int> counts;
    std::set<const Node *> exclude;

    get_neighbors(start, Any, strings.knows, false)
        .process([start_time, end_time, &counts, &exclude](Node &friend_node) {
            get_neighbors(friend_node, Incoming, strings.hasCreator, false)
                .process([start_time, end_time, &counts, &exclude](const Node &post) {
                    if (post.get_tag() == strings.Post) {
                        Time t = post.get_property(strings.creationDate).time_value();
                        if (t >= end_time)
                            return;
                        get_neighbors(post, Outgoing, strings.hasTag, false)
                            .process([start_time, t, &counts, &exclude](const Node &tag) {
                                if (exclude.find(&tag) == exclude.end()) {
                                    if (t < start_time) {
                                        counts.erase(&tag);
                                        exclude.insert(&tag);
                                    }
                                    else
                                        counts[&tag]++;
                                }
                            });
                    }
                });
        });


    Result result(counts.size());
    for (const auto &i : counts) {
        std::string name = i.first->get_property(strings.name).string_value();
        result.add(name, i.second);
    }

    result.sort_and_truncate();

    return result.get_results();
}

bool Query4ResultItem::operator<(const Query4ResultItem &a) const
{
    if (count == a.count)
        return tag < a.tag;
    return count > a.count;
}


#ifdef MAIN
#include <stdio.h>
#include "args.h"
#include "timing.h"

static void print(const Query4Result &results);
static void print(const Query4ResultItem &r);

strings_t strings;

int main(int argc, char **argv)
{
    GET_ARGS

    FILE *f = fopen(filename, "r");
    assert(f != NULL);

    char s[40];
    char *r = fgets(s, sizeof s, f);
    assert(r != NULL);
    assert(strcmp(s, "Person|Date0|Duration\n") == 0);

    try {
        Graph db(graph_name, Graph::ReadOnly);

        START_TIMING

        while (fgets(s, sizeof s, f) != NULL) {
            Transaction tx(db);
            char *e;
            unsigned long long id = strtoull(s, &e, 10);
            assert(*e == '|');
            unsigned long long start = strtoull(e + 1, &e, 10);
            assert(*e == '|');
            unsigned long long duration = strtoull(e + 1, &e, 10);
            assert(*e == '\n');
            time_t start_date = start / 1000; // convert ms to sec
            time_t end_date = start_date + duration * 24 * 3600; // convert days to sec
            if (verbose) {
                char start_str[40], end_str[40];
                strftime(start_str, sizeof start_str,
                         "%a %b %d %H:%M:%S %Y", gmtime(&start_date));
                strftime(end_str, sizeof end_str,
                         "%a %b %d %H:%M:%S %Y", gmtime(&end_date));
                printf("%llu %s %s\n", id, start_str, end_str);
            }

            Query4Result result = query4(db, id, start_date, end_date);

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

static void print(const Query4Result &results)
{
    if (results.empty())
        printf("  no matches\n");
    else
        for (const auto &r : results)
            print(r);
    printf("\n");
}

static void print(const Query4ResultItem &r)
{
    printf("  %s %d\n", r.tag.c_str(), r.count);
}
#endif
