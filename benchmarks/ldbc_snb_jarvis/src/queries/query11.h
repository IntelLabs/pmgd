//
// LDBC query 11
//
#include <string>
#include <list>
#include "pmgd.h"

struct Query11ResultItem {
    int year;
    unsigned long long person_id;
    std::string company_name;

    std::string firstName;
    std::string lastName;

    Query11ResultItem(int year, const PMGD::Node &person, const PMGD::Node &company);
    bool operator<(const Query11ResultItem &) const;
    void fill();

private:
    const PMGD::Node &_person;
};

typedef std::list<Query11ResultItem> Query11Result;

Query11Result query11(PMGD::Graph &db, unsigned long long id,
                      const char *country, int year);
