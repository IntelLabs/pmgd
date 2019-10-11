//
// LDBC query 7
//
#include <string>
#include <list>
#include "pmgd.h"

struct Query7ResultItem {
    unsigned long long person_id;
    PMGD::Time like_time;
    std::string firstName;
    std::string lastName;
    unsigned long long message_id;
    std::string content;
    int latency;
    bool isNew;

    Query7ResultItem(const PMGD::Node &person, PMGD::Time time, const PMGD::Node &post);
    bool operator<(const Query7ResultItem &a) const;
    void fill(const PMGD::Node &start);

private:
    const PMGD::Node &_person;
    const PMGD::Node &_post;
};

typedef std::list<Query7ResultItem> Query7Result;

Query7Result query7(PMGD::Graph &db, unsigned long long id);
