//
// LDBC query 14
//
#include <vector>
#include "pmgd.h"

struct Query14ResultItem {
    std::vector<unsigned long long> nodes;
    double weight;
};

typedef std::vector<Query14ResultItem> Query14Result;

Query14Result query14(PMGD::Graph &db, unsigned long long id1, unsigned long long id2);
