//
// LDBC query 5
//
#include <string>
#include <vector>
#include "pmgd.h"

struct Query5ResultItem {
    unsigned long long forum_id;
    int count;
    std::string forum_name;

    bool operator<(const Query5ResultItem &) const;
};

typedef std::vector<Query5ResultItem> Query5Result;

Query5Result query5(PMGD::Graph &db, unsigned long long id, time_t date);
