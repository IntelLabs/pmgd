//
// LDBC query 6
//
#include <string>
#include <vector>
#include "pmgd.h"

struct Query6ResultItem {
    std::string name;
    int count;

    bool operator<(const Query6ResultItem &) const;
};

typedef std::vector<Query6ResultItem> Query6Result;

Query6Result query6(PMGD::Graph &db, unsigned long long id, const char *tag_name);
