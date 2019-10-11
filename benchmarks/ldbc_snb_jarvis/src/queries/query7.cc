//
// LDBC query 7
//
#include <string>
#include <list>
#include "pmgd.h"
#include "util.h"
#include "neighbor.h"
#include "query7.h"
#include "strings.h"

using namespace PMGD;

class Result
{
    static const int MAX_RESULTS = 20;

    Query7Result results;

public:
    Query7Result get_results() { return results; }

    void add(const Node &person, Time like_time, const Node &post)
    {
        Query7ResultItem r(person, like_time, post);

        for (auto i = results.begin(); i != results.end(); i++) {
            if (r.person_id == i->person_id) {
                if (like_time > i->like_time) {
                    results.erase(i);
                    break;
                }

                return;
            }
        }

        if (results.size() < MAX_RESULTS || r < results.back()) {
            std::list<Query7ResultItem>::iterator pos = results.begin();
            while (pos != results.end() && *pos < r)
                pos++;
            results.insert(pos, r);

            if (results.size() > MAX_RESULTS)
                results.pop_back();
        }
    }

    void fill(const Node &start)
    {
        for (auto &r : results)
            r.fill(start);
    }
};


Query7Result query7(Graph &db, unsigned long long id)
{
    strings.init();

    Node &start = *db.get_nodes(strings.Person, PropertyPredicate(strings.id, PropertyPredicate::Eq, id));

    Result result;

    get_neighbors(start, Incoming, strings.hasCreator, false)
        .process([&result](const Node &post) {
            post.get_edges(Incoming, strings.likes)
                .process([&post, &result](const EdgeRef &e) {
                    const Node &person = e.get_source();
                    Time time = e.get_property(strings.creationDate).time_value();
                    result.add(person, time, post);
                });
        });

    result.fill(start);

    return result.get_results();
}


Query7ResultItem::Query7ResultItem(const Node &person, Time time, const Node &post)
    : _person(person), _post(post),
      person_id(person.get_property(strings.id).int_value()),
      like_time(time)
{ }

bool Query7ResultItem::operator<(const Query7ResultItem &a) const
{
    if (like_time == a.like_time)
        return person_id < a.person_id;
    return a.like_time < like_time;
}

void Query7ResultItem::fill(const Node &start)
{
    firstName = _person.get_property(strings.firstName).string_value();
    lastName = _person.get_property(strings.lastName).string_value();
    message_id = _post.get_property(strings.id).int_value();

    Property content_property;
    if (!(_post.check_property(strings.content, content_property)
            && (content = content_property.string_value()) != ""))
        content = _post.get_property(strings.imageFile).string_value();

    Time post_time = _post.get_property(strings.creationDate).time_value();
    uint64_t a = post_time.get_time_in_usec();
    uint64_t b = like_time.get_time_in_usec();

    latency = (b - a) / 60000000ull; // convert usec to min
    isNew = !bool(get_neighbors(start, Any, strings.knows, false)
                .filter([this](const Node &n) {
                    return Disposition(n == _person);
                }));
}

#ifdef MAIN
#include <stdio.h>
#include "args.h"
#include "timing.h"

static void print(const Query7Result &results);
static void print(const Query7ResultItem &r);

strings_t strings;


int main(int argc, char **argv)
{
    GET_ARGS

    FILE *f = fopen(filename, "r");
    assert(f != NULL);

    char s[120];
    char *r = fgets(s, sizeof s, f);
    assert(r != NULL);
    assert(strcmp(s, "Person\n") == 0);

    try {
        Graph db(graph_name, Graph::ReadOnly);

        START_TIMING

        while (fgets(s, sizeof s, f) != NULL) {
            Transaction tx(db);
            char *e;
            unsigned long long id = strtoull(s, &e, 10);
            assert(*e == '\n');
            if (verbose) printf("%llu\n", id);

            Query7Result result = query7(db, id);

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

static void print(const Query7Result &results)
{
    if (results.empty())
        printf("  no matches\n");
    else
        for (const auto &r : results)
            print(r);
    printf("\n");
}

static void print(const Query7ResultItem &r)
{
    printf("  %llu, %s %s%s, %s, %lld, %d min, %s\n",
            r.person_id, r.firstName.c_str(), r.lastName.c_str(),
            r.isNew ? " (new)" : "",
            time_to_string(r.like_time).c_str(), r.message_id,
            r.latency, r.content.c_str());
}
#endif
