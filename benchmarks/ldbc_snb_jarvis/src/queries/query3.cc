//
// LDBC query 3
//
#include <string>
#include <unordered_set>
#include <list>
#include "pmgd.h"
#include "util.h"
#include "neighbor.h"
#include "query3.h"
#include "strings.h"

using namespace PMGD;

static std::string get_country(const Node &n);

class Result
{
    static const int MAX_RESULTS = 20;

    Query3Result results;

public:
    Query3Result get_results() { return results; }

    void add(const Node &person, int count_x, int count_y)
    {
        Query3ResultItem r(person, count_x, count_y);

        if (results.size() < MAX_RESULTS || r < results.back()) {
            std::list<Query3ResultItem>::iterator pos = results.begin();
            while (pos != results.end() && *pos < r)
                pos++;
            results.insert(pos, r);

            if (results.size() > MAX_RESULTS)
                results.pop_back();
        }
    }
};


Query3Result query3(Graph &db, unsigned long long id,
                           time_t start_date, time_t end_date,
                           const char *country1, const char *country2)
{
    strings.init();

    PMGD::Time start_time(gmtime(&start_date), 0, 0);
    PMGD::Time end_time(gmtime(&end_date), 0, 0);

    Node &start = *db.get_nodes(strings.Person, PropertyPredicate(strings.id, PropertyPredicate::Eq, id));

    Result result;

    auto country_filter = [country1, country2](const Node &n) {
        std::string country = get_country(n);
        return Disposition(country != country1 && country != country2);
    };

    auto p = [country1, country2, start_time, end_time, &result](Node &person) {
        int count1 = 0, count2 = 0;
        NodeIterator posts = get_neighbors(person, Incoming, strings.hasCreator, false)
            .filter([start_time, end_time](const Node &post) {
                Time t = post.get_property(strings.creationDate).time_value();
                return Disposition(t >= start_time && t < end_time);
            });
        for (; posts; posts.next()) {
            std::string country = get_country(*posts);
            if (country == country1)
                count1++;
            if (country == country2)
                count2++;
        }
        if (count1 > 0 && count2 > 0)
            result.add(person, count1, count2);
    };

    get_neighborhood(start, 2, strings.knows, true)
        .filter(country_filter)
        .process(p);

    return result.get_results();
}


Query3ResultItem::Query3ResultItem(const Node &person, int count1, int count2)
{
    friend_id = person.get_property(strings.id).int_value();
    firstName = person.get_property(strings.firstName).string_value();
    lastName = person.get_property(strings.lastName).string_value();
    count_x = count1;
    count_y = count2;
    count = count1 + count2;
}

bool Query3ResultItem::operator<(const Query3ResultItem &a) const
{
    if (count == a.count)
        return friend_id < a.friend_id;
    return count > a.count;
}

static std::string get_country(const Node &n)
{
    Node &tmp = n.get_neighbor(Outgoing, strings.isLocatedIn);
    Node &place = tmp.get_property(strings.type).string_value() == "country"
                      ? tmp : tmp.get_neighbor(Outgoing, strings.isPartOf);
    return place.get_property(strings.name).string_value();
}

#ifdef MAIN
#include <stdio.h>
#include "args.h"
#include "timing.h"

static void print(const Query3Result &);
static void print(const Query3ResultItem &);

strings_t strings;

int main(int argc, char **argv)
{
    GET_ARGS

    FILE *f = fopen(filename, "r");
    assert(f != NULL);

    char s[120];
    char *r = fgets(s, sizeof s, f);
    assert(r != NULL);
    assert(strcmp(s, "Person|Date0|Duration|Country1|Country2\n") == 0);

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
            assert(*e == '|');
            char *country1 = e + 1;
            e = strchr(country1, '|');
            assert(e);
            *e = '\0';
            char *country2 = e + 1;
            e = strchr(country2, '\n');
            assert(e);
            *e = '\0';
            time_t start_date = start / 1000; // convert ms to sec
            time_t end_date = start_date + duration * 24 * 3600; // convert days to sec
            if (verbose) {
                char start_str[40], end_str[40];
                strftime(start_str, sizeof start_str,
                         "%a %b %d %H:%M:%S %Y", gmtime(&start_date));
                strftime(end_str, sizeof end_str,
                         "%a %b %d %H:%M:%S %Y", gmtime(&end_date));
                printf("%llu %s %s %s %s\n", id, start_str, end_str, country1, country2);
            }

            Query3Result result = query3(db, id, start_date, end_date, country1, country2);

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

void print(const Query3Result &results)
{
    if (results.empty())
        printf("  no matches\n");
    else
        for (const auto &r : results)
            print(r);
    printf("\n");
}

void print(const Query3ResultItem &r)
{
    printf("  %llu, %s %s, %d, %d, %d\n",
            r.friend_id, r.firstName.c_str(), r.lastName.c_str(),
            r.count_x, r.count_y, r.count);
}
#endif
