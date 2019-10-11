//
// LDBC query 10
//
#include <string>
#include <list>
#include <unordered_set>
#include "pmgd.h"
#include "util.h"
#include "neighbor.h"
#include "query10.h"
#include "strings.h"

using namespace PMGD;

class Result
{
    static const int MAX_RESULTS = 10;

    Query10Result results;

public:
    Query10Result get_results() { return results; }

    void add(const Node &person, int similarity)
    {
        Query10ResultItem r(person, similarity);

        if (results.size() < MAX_RESULTS || r < results.back()) {
            std::list<Query10ResultItem>::iterator pos = results.begin();
            while (pos != results.end() && *pos < r)
                pos++;
            results.insert(pos, r);

            if (results.size() > MAX_RESULTS)
                results.pop_back();
        }
    }

    void fill()
    {
        for (auto &r : results)
            r.fill();
    }
};


typedef std::unordered_set<const Node *> interests_t;
static int compute_similarity(const interests_t &interests, const Node &person);

Query10Result query10(Graph &db, unsigned long long id, int month)
{
    strings.init();

    Node &start = *db.get_nodes(strings.Person, PropertyPredicate(strings.id, PropertyPredicate::Eq, id));

    interests_t interests;
    get_neighbors(start, Outgoing, strings.hasInterest, false)
        .process([&interests](const Node &tag) {
            interests.insert(&tag);
        });

    Result result;

    std::unordered_set<const Node *> encountered;
    encountered.insert(&start);

    auto e = [&encountered](const Node &n) {
        return Disposition(encountered.insert(&n).second);
    };

    get_neighbors(start, Any, strings.knows, false)
        .process([&encountered](const Node &n) { encountered.insert(&n); });

    get_nhop_neighbors(start, 2, Any, strings.knows)
        .filter(e)
        .filter([month](const Node &n) {
            struct tm tm;
            n.get_property(strings.birthday).time_value().get_utc(&tm);
            int birth_month = tm.tm_mon + 1;
            return Disposition((birth_month == month && tm.tm_mday >= 21)
                || ((birth_month == month + 1 || month == 12 && birth_month == 1)
                        && tm.tm_mday < 22));
        })
        .process([&interests, &result](const Node &person) {
            result.add(person, compute_similarity(interests, person));
        });

    result.fill();

    return result.get_results();
}

static int compute_similarity(const interests_t &interests, const Node &person)
{
    int similarity = 0;

    get_neighbors(person, Incoming, strings.hasCreator, false)
        .filter([](const Node &post) {
            return Disposition(post.get_tag() == strings.Post);
        })
        .process([&interests, &similarity](const Node &post) {
            bool common;
            common = get_neighbors(post, Outgoing, strings.hasTag, false)
                 .filter([&interests, &similarity](const Node &tag) {
                     return Disposition(interests.find(&tag) != interests.end());
                 });
            if (common)
                similarity++;
            else
                similarity--;
        });

    return similarity;
}


Query10ResultItem::Query10ResultItem(const Node &person, int s)
    : _person(person),
      similarity(s),
      person_id(person.get_property(strings.id).int_value())
{
}

void Query10ResultItem::fill()
{
    firstName = _person.get_property(strings.firstName).string_value();
    lastName = _person.get_property(strings.lastName).string_value();
    gender = _person.get_property(strings.gender).string_value();
    place = _person.get_neighbor(Outgoing, strings.isLocatedIn)
                .get_property(strings.name).string_value();
}

bool Query10ResultItem::operator<(const Query10ResultItem &a) const
{
    if (similarity == a.similarity)
        return person_id < a.person_id;
    return similarity > a.similarity;
}


#ifdef MAIN
#include <stdio.h>
#include "args.h"
#include "timing.h"

static void print(const Query10Result &results);
static void print(const Query10ResultItem &r);

strings_t strings;

int main(int argc, char **argv)
{
    GET_ARGS

    FILE *f = fopen(filename, "r");
    assert(f != NULL);

    char s[40];
    char *r = fgets(s, sizeof s, f);
    assert(r != NULL);
    assert(strcmp(s, "Person|HS0\n") == 0);

    try {
        Graph db(graph_name, Graph::ReadOnly);

        START_TIMING

        while (fgets(s, sizeof s, f) != NULL) {
            Transaction tx(db);
            char *e;
            unsigned long long id = strtoull(s, &e, 10);
            assert(*e == '|');
            int month = strtoull(e + 1, &e, 10);
            assert(*e == '\n');
            if (verbose) printf("%llu %d\n", id, month);

            Query10Result result = query10(db, id, month);

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

static void print(const Query10Result &results)
{
    if (results.empty())
        printf("  no matches\n");
    else
        for (const auto &r : results)
            print(r);
    printf("\n");
}

static void print(const Query10ResultItem &r)
{
    printf("  %d, %llu, %s %s, %s, %s\n",
            r.similarity, r.person_id, r.firstName.c_str(), r.lastName.c_str(),
            r.gender.c_str(), r.place.c_str());
}
#endif
