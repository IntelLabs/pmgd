//
// LDBC query 1
//
#include <string>
#include <unordered_set>
#include "pmgd.h"
#include "util.h"
#include "neighbor.h"
#include "query1.h"
#include "strings.h"

using namespace PMGD;

strings_t strings;

class Result
{
    static const int MAX_RESULTS = 20;

    Query1Result results;

public:
    Query1Result get_results() { return results; }

    void add(const Node &person, int distance)
    {
        Query1ResultItem r(person, distance);

        if (results.size() < MAX_RESULTS || r < results.back()) {
            Query1Result::iterator pos = results.begin();
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

static std::string get_place(const Node &n);
static std::string get_city(const Node &n);
static std::string get_country(const Node &n);

Query1Result query1(Graph &db, unsigned long long id, const char *name)
{
    strings.init();

    Node &start = *db.get_nodes(strings.Person,
                      PropertyPredicate(strings.id, PropertyPredicate::Eq, id));
    Property first_name(name);
    Result result;

    NeighborhoodIterator i = get_neighborhood(start, 3, strings.knows, true);
    while (i) {
        Property p;
        if (i->check_property(strings.firstName, p) && p == first_name)
            result.add(*i, i.distance());
        i.next();
    }

    result.fill();

    return result.get_results();
}


Query1ResultItem::Query1ResultItem(const Node &person, int d)
    : _person(person),
      distance(d)
{
    id = person.get_property(strings.id).int_value();
    lastName = person.get_property(strings.lastName).string_value();
}

void Query1ResultItem::fill()
{
    birthday = _person.get_property(strings.birthday).time_value();
    creationDate = _person.get_property(strings.creationDate).time_value();
    gender = _person.get_property(strings.gender).string_value();
    browserUsed = _person.get_property(strings.browserUsed).string_value();
    locationIP = _person.get_property(strings.locationIP).string_value();

    _person.get_edges(Outgoing, strings.email)
        .process([this](EdgeRef &e)
            { emailaddresses.push_back(e.get_property(strings.emailaddress).string_value()); });

    get_neighbors(_person, Outgoing, strings.speaks, false)
        .process([this](Node &n)
            { languages.push_back(n.get_property(strings.language).string_value()); });

    place = get_place(_person);
    _person.get_edges(Outgoing, strings.studyAt)
        .process([this](EdgeRef &e) {
            const Node &n = e.get_destination();
            universities.push_back(University{
                                     n.get_property(strings.name).string_value(),
                                     int(e.get_property(strings.classYear).int_value()),
                                     get_city(n) });
        });

    _person.get_edges(Outgoing, strings.workAt)
        .process([this](EdgeRef &e) {
            const Node &n = e.get_destination();
            companies.push_back(Company{
                                     n.get_property(strings.name).string_value(),
                                     int(e.get_property(strings.workFrom).int_value()),
                                     get_country(n) });
        });
}

bool Query1ResultItem::operator<(const Query1ResultItem &a) const
{
    if (distance != a.distance)
        return distance < a.distance;
    int r = lastName.compare(a.lastName);
    if (r != 0)
        return r < 0;
    return id < a.id;
}

static std::string get_place(const Node &n)
{
    return n.get_neighbor(Outgoing, strings.isLocatedIn).get_property(strings.name).string_value();
}

static std::string get_city(const Node &n)
{
    Node &place = n.get_neighbor(Outgoing, strings.isLocatedIn);
    assert(place.get_property(strings.type).string_value() == "city");
    return place.get_property(strings.name).string_value();
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
#include <algorithm>
#include "args.h"
#include "timing.h"

static void print(const Query1Result &);
static void print(const Query1ResultItem &);

int main(int argc, char **argv)
{
    GET_ARGS

    FILE *f = fopen(filename, "r");
    assert(f != NULL);

    char s[40];
    char *r = fgets(s, sizeof s, f);
    assert(r != NULL);
    assert(strcmp(s, "Person|Name\n") == 0);

    try {
        Graph db(graph_name, Graph::ReadOnly);

        START_TIMING

        char s[60];
        while (fgets(s, sizeof s, f) != NULL) {
            Transaction tx(db);
            char *name;
            unsigned long long id = strtoull(s, &name, 10);
            assert(*name == '|');
            name++;
            int l = strlen(name);
            assert(name[l - 1] == '\n');
            name[l - 1] = '\0';
            if (verbose) printf("%llu %s\n", id, name);

            Query1Result result = query1(db, id, name);

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

static void print(const Query1Result &results)
{
    if (results.empty())
        printf("  no matches\n");
    else
        for (const auto &r : results)
            print(r);
    printf("\n");
}

static void print(const Query1ResultItem &r)
{
    char birthday_str[40], creationDate_str[40];
    struct tm birthday_tm, creationDate_tm;
    r.birthday.get_tm(&birthday_tm);
    r.creationDate.get_tm(&creationDate_tm);
    strftime(birthday_str, sizeof birthday_str, "%F", &birthday_tm);
    strftime(creationDate_str, sizeof creationDate_str,
             "%a %b %d %H:%M:%S %Y", &creationDate_tm);

    printf("  %d, %llu, %s, %s, %s, %s, %s, %s\n",
            r.distance, r.id, r.lastName.c_str(), birthday_str,
            creationDate_str, r.gender.c_str(), r.browserUsed.c_str(),
            r.locationIP.c_str());

    std::vector<std::string> emailaddresses = r.emailaddresses;
    std::sort(emailaddresses.begin(), emailaddresses.end());
    for (const auto &e : emailaddresses)
        printf("    %s\n", e.c_str());

    std::vector<std::string> languages = r.languages;
    std::sort(languages.begin(), languages.end());
    bool b = false;
    for (const auto &l : languages) {
        if (!b) { printf("    "); b = true; } else printf(", ");
        printf("%s", l.c_str());
    }
    if (b) printf("\n");

    printf("    %s\n", r.place.c_str());

    std::vector<Query1ResultItem::University> universities = r.universities;
    std::sort(universities.begin(), universities.end());
    for (const auto &u : universities)
        printf("    %s, %d, %s\n", u.name.c_str(), u.year, u.city.c_str());

    std::vector<Query1ResultItem::Company> companies = r.companies;
    std::sort(companies.begin(), companies.end());
    for (const auto &c : companies)
        printf("    %s, %d, %s\n", c.name.c_str(), c.year, c.country.c_str());
}
#endif
