//
// LDBC query 12
//
#include <string>
#include <list>
#include <set>
#include "pmgd.h"

struct Query12ResultItem {
    const unsigned long long person_id;
    int count;
    std::set<std::string> tags;

    std::string firstName;
    std::string lastName;

    Query12ResultItem(const PMGD::Node &person);
    bool operator<(const Query12ResultItem &) const;
    void fill();

private:
    const PMGD::Node &_person;
};

typedef std::list<Query12ResultItem> Query12Result;

Query12Result query12(PMGD::Graph &db, unsigned long long id, const char *tag_class_name);
