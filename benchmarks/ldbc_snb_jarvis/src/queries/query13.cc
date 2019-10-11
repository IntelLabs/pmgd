//
// LDBC query 13
//
#include <unordered_set>
#include "pmgd.h"
#include "util.h"
#include "neighbor.h"
#include "query13.h"
#include "strings.h"

using namespace PMGD;

Query13Result query13(Graph &db, unsigned long long id1, unsigned long long id2)
{
    strings.init();

    if (id1 == id2)
        return 0;

    const Node &n1 = *db.get_nodes(strings.Person, PropertyPredicate(strings.id, PropertyPredicate::Eq, id1));
    const Node &n2 = *db.get_nodes(strings.Person, PropertyPredicate(strings.id, PropertyPredicate::Eq, id2));

    std::unordered_set<const Node *> neighbors({&n1});
    std::unordered_set<const Node *> encountered({&n1});

    int distance = 1;
    while (!neighbors.empty()) {
        std::unordered_set<const Node *> new_neighbors;
        for (const auto p : neighbors) {
            NodeIterator i = get_neighbors(*p, Any, strings.knows, false);
            for ( ; i; i.next()) {
                const Node &neighbor = *i;
                if (neighbor == n2)
                    return distance;
                if (encountered.insert(&neighbor).second)
                    new_neighbors.insert(&neighbor);
            }
        }

        neighbors = move(new_neighbors);
        distance++;
    }

    return -1;
}


#ifdef MAIN
#include <stdio.h>
#include "args.h"
#include "timing.h"

static void print(const Query13Result &result);

strings_t strings;

int main(int argc, char **argv)
{
    GET_ARGS

    FILE *f = fopen(filename, "r");
    assert(f != NULL);

    char s[40];
    char *r = fgets(s, sizeof s, f);
    assert(r != NULL);
    assert(strcmp(s, "Person1|Person2\n") == 0);

    try {
        Graph db(graph_name, Graph::ReadOnly);

        START_TIMING

        char s[40];
        while (fgets(s, sizeof s, f) != NULL) {
            Transaction tx(db);
            char *e;
            unsigned long long id1 = strtoull(s, &e, 10);
            assert(*e == '|');
            unsigned long long id2 = strtoull(e + 1, &e, 10);
            assert(*e == '\n');
            if (verbose) printf("%llu %llu\n", id1, id2);

            Query13Result result = query13(db, id1, id2);

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

static void print(const Query13Result &result)
{
    printf("  %d\n", result);
}
#endif
