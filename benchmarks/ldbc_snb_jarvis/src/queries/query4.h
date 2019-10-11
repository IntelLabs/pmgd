//
// LDBC query 4
//
#include <string>
#include <vector>
#include "pmgd.h"

struct Query4ResultItem {
    std::string tag;
    int count;

    bool operator<(const Query4ResultItem &) const;
};

typedef std::vector<Query4ResultItem> Query4Result;

Query4Result query4(PMGD::Graph &db, unsigned long long id, time_t start, time_t end);
