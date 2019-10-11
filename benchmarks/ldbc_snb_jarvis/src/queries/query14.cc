//
// LDBC query 14
//
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include "pmgd.h"
#include "util.h"
#include "neighbor.h"
#include "query14.h"
#include "strings.h"

using namespace PMGD;

class Result {
    struct Path {
        std::vector<const Node *> nodes;
        unsigned long weight;  // scaled by 1 bit fractional part

        Path(const Node &n) { nodes.push_back(&n); }
        Path(const Path &p, const Node &n)
        {
            nodes.reserve(p.nodes.size() + 1);
            nodes = p.nodes;
            nodes.push_back(&n);
        }
        const Node &end() const { return *nodes[nodes.size() - 1]; }
        bool operator<(const Path &) const;
    };

    std::vector<Path> results;

    static unsigned long compute_weight(const Node &n1, const Node &n2);

public:
    Query14Result get_results();
    void shortest_paths(const Node &n1, const Node &n2);
    void compute_weights();
    void sort() { std::sort(results.begin(), results.end()); }
};


Query14Result query14(Graph &db, unsigned long long id1, unsigned long long id2)
{
    strings.init();

    const Node &n1 = *db.get_nodes(strings.Person, PropertyPredicate(strings.id, PropertyPredicate::Eq, id1));
    const Node &n2 = *db.get_nodes(strings.Person, PropertyPredicate(strings.id, PropertyPredicate::Eq, id2));

    Result result;
    result.shortest_paths(n1, n2);
    result.compute_weights();
    result.sort();

    return result.get_results();
}

void Result::shortest_paths(const Node &n1, const Node &n2)
{
    std::vector<Path> paths;
    std::unordered_set<const Node *> encountered;
    encountered.insert(&n1);
    paths.push_back(Path(n1));

    while (1) {
        std::vector<const Node *> new_neighbors;
        std::vector<Path> new_paths;
        for (const auto &p : paths) {
            get_neighbors(p.end(), Any, strings.knows, false)
                .process([&p, &encountered, &n2, &new_neighbors, &new_paths, this]
                        (const Node &neighbor)
                {
                    if (encountered.find(&neighbor) == encountered.end()) {
                        Path new_path(p, neighbor);
                        if (neighbor == n2)
                            results.push_back(new_path);
                        else  {
                            new_neighbors.push_back(&neighbor);
                            new_paths.push_back(new_path);
                        }
                    }
                });
        }

        if (!results.empty())
            break;
        if (new_paths.empty())
            break;
        paths = move(new_paths);
        encountered.insert(new_neighbors.begin(), new_neighbors.end());
    }
}


void Result::compute_weights()
{
    for (auto &p : results) {
        p.weight = 0;
        for (int i = 0; i < p.nodes.size() - 1; i++)
            p.weight += compute_weight(*p.nodes[i], *p.nodes[i+1]);
    }
}


unsigned long Result::compute_weight(const Node &n1, const Node &n2)
{
    unsigned long weight = 0;

    struct p {
        const Node &_n;
        unsigned long &_weight;

        p(const Node &n, unsigned long &w) : _n(n), _weight(w) { }

        void operator()(Node &comment) {
            NodeIterator tmp = get_neighbors(comment, Outgoing, strings.replyOf);
            if (!tmp)
                return;
            const Node &post = *tmp;
            if (post.get_neighbor(Outgoing, strings.hasCreator) == _n)
                if (post.get_tag() == strings.Post)
                    _weight += 2; // 1.0 scaled by 1 bit
                else
                    _weight += 1; // 0.5 scaled by 1 bit
        };
    };

    get_neighbors(n1, Incoming, strings.hasCreator, false)
        .process(p(n2, weight));

    get_neighbors(n2, Incoming, strings.hasCreator, false)
        .process(p(n1, weight));

    return weight;
}

bool Result::Path::operator<(const Path &a) const
{
    return weight > a.weight;
}

Query14Result Result::get_results()
{
    Query14Result result;
    for (const auto &r : results) {
        Query14ResultItem rr;
        rr.weight = (double)r.weight / 2;
        for (const auto n : r.nodes)
            rr.nodes.push_back(n->get_property(strings.id).int_value());
        result.push_back(rr);
    }
    return result;
}


#ifdef MAIN
#include <stdio.h>
#include "args.h"
#include "timing.h"

static void print(const Query14Result &);
static void print(const Query14ResultItem &);

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

            Query14Result result = query14(db, id1, id2);

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

static void print(const Query14Result &results)
{
    if (results.empty())
        printf("  no matches\n");
    else {
        Query14Result resort = results;
        std::sort(resort.begin(), resort.end(),
                  [](const Query14ResultItem &a, const Query14ResultItem &b) {
                      if (a.weight != b.weight)
                          return a.weight > b.weight;
                      assert(a.nodes.size() == b.nodes.size());
                      for (int i = 0; i < a.nodes.size(); i++)
                          if (a.nodes[i] != b.nodes[i])
                              return a.nodes[i] < b.nodes[i];
                      assert(0);
                  });

        for (const auto &r : resort)
            print(r);
    }
    printf("\n");
}

static void print(const Query14ResultItem &r)
{
    printf("  %.1f", r.weight);
    for (const auto id : r.nodes)
        printf(", %llu", id);
    printf("\n");
}
#endif
