//
// LDBC query 9
//
#include <string>
#include <list>
#include "pmgd.h"

struct Query9ResultItem {
    PMGD::Time creationDate;
    unsigned long long post_id;
    unsigned long long friend_id;
    std::string firstName;
    std::string lastName;
    std::string content;

    Query9ResultItem(const PMGD::Node &person, const PMGD::Node &post);
    bool operator<(const Query9ResultItem &) const;
    void fill();

private:
    const PMGD::Node &_person;
    const PMGD::Node &_post;
};

typedef std::list<Query9ResultItem> Query9Result;

Query9Result query9(PMGD::Graph &db, unsigned long long id, time_t date);
