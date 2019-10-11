//
// LDBC query 8
//
#include <string>
#include <list>
#include "pmgd.h"
#include "util.h"
#include "neighbor.h"
#include "query8.h"
#include "strings.h"

using namespace PMGD;

class Result
{
    static const int MAX_RESULTS = 20;

    Query8Result results;

public:
    Query8Result get_results() { return results; }

    void add(const Node &comment)
    {
        Query8ResultItem r(comment);

        if (results.size() < MAX_RESULTS || r < results.back()) {
            std::list<Query8ResultItem>::iterator pos = results.begin();
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


Query8Result query8(Graph &db, unsigned long long id)
{
    strings.init();

    Node &start = *db.get_nodes(strings.Person, PropertyPredicate(strings.id, PropertyPredicate::Eq, id));

    Result result;

    get_neighbors(start, Incoming, strings.hasCreator, false)
        .process([&result](const Node &post) {
            get_neighbors(post, Incoming, strings.replyOf, false)
                .process([&post, &result](const Node &comment) {
                    result.add(comment);
                });
        });

    result.fill(start);

    return result.get_results();
}


Query8ResultItem::Query8ResultItem(const Node &comment)
    : _comment(comment),
      comment_time(comment.get_property(strings.creationDate).time_value()),
      comment_id(comment.get_property(strings.id).int_value())
{ }

bool Query8ResultItem::operator<(const Query8ResultItem &a) const
{
    if (comment_time == a.comment_time)
        return comment_id < a.comment_id;
    return a.comment_time < comment_time;
}

void Query8ResultItem::fill(const Node &start)
{
    const Node &commenter = _comment.get_neighbor(Outgoing, strings.hasCreator);
    commenter_id = commenter.get_property(strings.id).int_value();
    firstName = commenter.get_property(strings.firstName).string_value();
    lastName = commenter.get_property(strings.lastName).string_value();

    Property content_property;
    if (!(_comment.check_property(strings.content, content_property)
            && (content = content_property.string_value()) != ""))
        content = _comment.get_property(strings.imageFile).string_value();
}


#ifdef MAIN
#include <stdio.h>
#include "args.h"
#include "timing.h"

static void print(const Query8Result &results);
static void print(const Query8ResultItem &r);

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

            Query8Result result = query8(db, id);

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

static void print(const Query8Result &results)
{
    if (results.empty())
        printf("  no matches\n");
    else
        for (const auto &r : results)
            print(r);
    printf("\n");
}

static void print(const Query8ResultItem &r)
{
    printf("  %llu, %s %s, %s, %lld, %s\n",
            r.commenter_id, r.firstName.c_str(), r.lastName.c_str(),
            time_to_string(r.comment_time).c_str(), r.comment_id,
            r.content.c_str());
}
#endif
