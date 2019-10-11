//
// LDBC query 2
//
#include <string>
#include <list>
#include "pmgd.h"

struct Query2ResultItem {
    unsigned long long post_id;
    PMGD::Time creationDate;

    unsigned long long friend_id;
    std::string firstName;
    std::string lastName;
    std::string content;

    Query2ResultItem(const PMGD::Node &person, const PMGD::Node &post);
    bool operator<(const Query2ResultItem &) const;
    void fill();

private:
    const PMGD::Node &_person;
    const PMGD::Node &_post;
};

typedef std::list<Query2ResultItem> Query2Result;

Query2Result query2(PMGD::Graph &db, unsigned long long id, unsigned long long date);
