//
// LDBC query 1
//
#include <string>
#include <vector>
#include <list>
#include "pmgd.h"

struct Query1ResultItem {
    struct University {
        std::string name;
        int year;
        std::string city;
        bool operator<(const University &a) const { return name < a.name; }
    };
    struct Company {
        std::string name;
        int year;
        std::string country;
        bool operator<(const Company &a) const { return name < a.name; }
    };

    int distance;
    std::string lastName;
    unsigned long long id;

    PMGD::Time birthday;
    PMGD::Time creationDate;
    std::string gender;
    std::string browserUsed;
    std::string locationIP;
    std::vector<std::string> emailaddresses;
    std::vector<std::string> languages;
    std::string place;
    std::vector<University> universities;
    std::vector<Company> companies;

    Query1ResultItem(const PMGD::Node &person, int distance);
    bool operator<(const Query1ResultItem &) const;
    void fill();

private:
    const PMGD::Node &_person;
};

typedef std::list<Query1ResultItem> Query1Result;

Query1Result query1(PMGD::Graph &db, unsigned long long id, const char *name);
