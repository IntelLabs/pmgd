//
// LDBC query 10
//
#include <string>
#include <list>
#include "pmgd.h"

struct Query10ResultItem {
    const int similarity;
    const unsigned long long person_id;

    std::string firstName;
    std::string lastName;
    std::string gender;
    std::string place;

    Query10ResultItem(const PMGD::Node &person, int similarity);
    bool operator<(const Query10ResultItem &) const;
    void fill();

private:
    const PMGD::Node &_person;
};

typedef std::list<Query10ResultItem> Query10Result;

Query10Result query10(PMGD::Graph &db, unsigned long long id, int month);
