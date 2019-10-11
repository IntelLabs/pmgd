//
// LDBC query 8
//
#include <string>
#include <list>
#include "pmgd.h"

struct Query8ResultItem {
    PMGD::Time comment_time;
    unsigned long long comment_id;
    unsigned long long commenter_id;
    std::string firstName;
    std::string lastName;
    std::string content;

    Query8ResultItem(const PMGD::Node &comment);
    bool operator<(const Query8ResultItem &a) const;
    void fill(const PMGD::Node &start);

private:
    const PMGD::Node &_comment;
};

typedef std::list<Query8ResultItem> Query8Result;

Query8Result query8(PMGD::Graph &db, unsigned long long id);
