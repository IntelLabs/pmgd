//
// LDBC query 12
//
#include <string>
#include <vector>
#include <list>
#include <set>
#include "pmgd.h"
#include "util.h"
#include "neighbor.h"
#include "query12.h"
#include "strings.h"

using namespace PMGD;

struct Result
{
    static const int MAX_RESULTS = 20;

    Query12Result results;

public:
    Query12Result get_results() { return results; }

    void add(Query12ResultItem &r)
    {
        if (results.size() < MAX_RESULTS || r < results.back()) {
            std::list<Query12ResultItem>::iterator pos = results.begin();
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


static bool check_tag(const Node &tag, const char *tag_class_name);
static bool check_tag_class(const Node &tag_class, const char *tag_class_name);

Query12Result query12(Graph &db, unsigned long long id, const char *tag_class_name)
{
    strings.init();

    Node &start = *db.get_nodes(strings.Person,
             PropertyPredicate(strings.id, PropertyPredicate::Eq, id));

    Result result;

    get_neighbors(start, Any, strings.knows, false)
        .process([tag_class_name, &result](const Node &person) {
            Query12ResultItem item(person);
            get_neighbors(person, Incoming, strings.hasCreator, false)
                .process([tag_class_name, &item](const Node &comment) {
                    NodeIterator tmp = get_neighbors(comment, Outgoing, strings.replyOf, false);
                    if (tmp) {
                        const Node &post = *tmp;
                        if (post.get_tag() == strings.Post) {
                            bool flag = false;
                            get_neighbors(post, Outgoing, strings.hasTag, false)
                                .process([tag_class_name, &flag, &item](const Node &tag) {
                                    if (check_tag(tag, tag_class_name)) {
                                        item.tags.insert(tag.get_property(strings.name).string_value());
                                        flag = true;
                                    }
                                });
                            if (flag)
                                item.count++;
                        }
                    }
                });

            if (item.count != 0)
                result.add(item);
        });

    result.fill();

    return result.get_results();
}

static bool check_tag(const Node &tag, const char *tag_class_name)
{
    NodeIterator i1 = get_neighbors(tag, Outgoing, strings.hasType, false);
    for ( ; i1; i1.next())
        if (check_tag_class(*i1, tag_class_name))
            return true;

    return false;
}

static bool check_tag_class(const Node &tag_class, const char *tag_class_name)
{
    if (tag_class.get_property(strings.name).string_value() == tag_class_name)
        return true;

    NodeIterator i2 = get_neighbors(tag_class, Outgoing, strings.isSubclassOf, false);
    for ( ; i2; i2.next())
        if (check_tag_class(*i2, tag_class_name))
            return true;

    return false;
}

Query12ResultItem::Query12ResultItem(const Node &person)
    : _person(person),
      count(0),
      person_id(person.get_property(strings.id).int_value())
{ }

void Query12ResultItem::fill()
{
    firstName = _person.get_property(strings.firstName).string_value();
    lastName = _person.get_property(strings.lastName).string_value();
}

bool Query12ResultItem::operator<(const Query12ResultItem &a) const
{
    if (count == a.count)
        return person_id < a.person_id;
    return count > a.count;
}


#ifdef MAIN
#include <stdio.h>
#include "args.h"
#include "timing.h"

static void print(const Query12Result &);
static void print(const Query12ResultItem &);

strings_t strings;

int main(int argc, char **argv)
{
    GET_ARGS

    FILE *f = fopen(filename, "r");
    assert(f != NULL);

    char s[80];
    char *r = fgets(s, sizeof s, f);
    assert(r != NULL);
    assert(strcmp(s, "Person|TagType\n") == 0);

    try {
        Graph db(graph_name, Graph::ReadOnly);

        START_TIMING

        while (fgets(s, sizeof s, f) != NULL) {
            Transaction tx(db);
            char *e;
            unsigned long long id = strtoull(s, &e, 10);
            assert(*e == '|');
            char *tag_class_name = e + 1;
            e = strchr(tag_class_name, '\n');
            assert(e != NULL);
            *e = '\0';
            if (verbose) printf("%llu %s\n", id, tag_class_name);

            Query12Result result = query12(db, id, tag_class_name);

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

static void print(const Query12Result &results)
{
    if (results.empty())
        printf("  no matches\n");
    else
        for (const auto &r : results)
            print(r);
    printf("\n");
}

static void print(const Query12ResultItem &r)
{
    printf("  %llu, %s %s, %d\n",
            r.person_id, r.firstName.c_str(), r.lastName.c_str(), r.count);
    for (const auto &t : r.tags)
        printf("    %s\n", t.c_str());
}
#endif
