//
// LDBC query 3
//
#include <string>
#include <list>
#include "pmgd.h"

struct Query3ResultItem {
    unsigned long long friend_id;
    std::string firstName;
    std::string lastName;
    int count_x;
    int count_y;
    int count;

    Query3ResultItem(const PMGD::Node &person, int count1, int count2);
    bool operator<(const Query3ResultItem &) const;
    void fill() { }
};

typedef std::list<Query3ResultItem> Query3Result;

Query3Result query3(PMGD::Graph &db, unsigned long long id,
                    time_t start_date, time_t end_date,
                    const char *country1, const char *country2);
