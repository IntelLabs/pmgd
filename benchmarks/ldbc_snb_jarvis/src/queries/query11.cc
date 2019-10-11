//
// LDBC query 11
//
#include <string>
#include <list>
#include <unordered_set>
#include "pmgd.h"
#include "util.h"
#include "neighbor.h"
#include "query11.h"
#include "strings.h"

using namespace PMGD;

class Result
{
    static const int MAX_RESULTS = 10;

    Query11Result results;

public:
    Query11Result get_results() { return results; }

    void add(int year, const Node &person, const Node &company)
    {
        Query11ResultItem r(year, person, company);

        if (results.size() < MAX_RESULTS || r < results.back()) {
            std::list<Query11ResultItem>::iterator pos = results.begin();
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

static std::string get_country(const Node &n);

Query11Result query11(Graph &db, unsigned long long id, const char *country, int year)
{
    strings.init();

    Node &start = *db.get_nodes(strings.Person, PropertyPredicate(strings.id, PropertyPredicate::Eq, id));

    Result result;

    auto p = [year, country, &result](Node &person) {
        person.get_edges(Outgoing, strings.workAt)
            .process([year, country, &person, &result](const EdgeRef &e) {
                int start_year = e.get_property(strings.workFrom).int_value();
                if (start_year >= year)
                    return;
                const Node &company = e.get_destination();
                if (get_country(company) != country)
                    return;
                result.add(start_year, person, company);
            });
    };

    get_neighborhood(start, 2, strings.knows, true)
        .process(p);

    result.fill();

    return result.get_results();
}

Query11ResultItem::Query11ResultItem(int start_year, const Node &person, const Node &company)
    : _person(person)
{
    year = start_year;
    person_id = person.get_property(strings.id).int_value();
    company_name = company.get_property(strings.name).string_value();
}

void Query11ResultItem::fill()
{
    firstName = _person.get_property(strings.firstName).string_value();
    lastName = _person.get_property(strings.lastName).string_value();
}

bool Query11ResultItem::operator<(const Query11ResultItem &a) const
{
    if (year == a.year)
        if (person_id == a.person_id)
            return company_name > a.company_name;
        else
            return person_id < a.person_id;
    else
        return year < a.year;
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

static void print(const Query11Result &);
static void print(const Query11ResultItem &);

strings_t strings;

int main(int argc, char **argv)
{
    GET_ARGS

    FILE *f = fopen(filename, "r");
    assert(f != NULL);

    char s[120];
    char *r = fgets(s, sizeof s, f);
    assert(r != NULL);
    assert(strcmp(s, "Person|Country|Year\n") == 0);

    try {
        Graph db(graph_name, Graph::ReadOnly);

        START_TIMING

        while (fgets(s, sizeof s, f) != NULL) {
            Transaction tx(db);
            char *e;
            unsigned long long id = strtoull(s, &e, 10);
            assert(*e == '|');
            char *country = e + 1;
            e = strchr(country, '|');
            assert(e != NULL);
            *e = '\0';
            int year = strtoul(e + 1, &e, 10);
            assert(*e == '\n');

            if (verbose) printf("%llu %s %d\n", id, country, year);

            Query11Result result = query11(db, id, country, year);

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

static void print(const Query11Result &results)
{
    if (results.empty())
        printf("  no matches\n");
    else
        for (const auto &r : results)
            print(r);
    printf("\n");
}

static void print(const Query11ResultItem &r)
{
    printf("  %llu, %s %s, %s, %d\n",
        r.person_id, r.firstName.c_str(), r.lastName.c_str(),
        r.company_name.c_str(), r.year);
}
#endif
