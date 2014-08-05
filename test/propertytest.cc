/*
 * This test checks Jarvis property lists
 */

#include <stdio.h>
#include "jarvis.h"
#include "../util/util.h"

using namespace Jarvis;

int main(int argc, char **argv)
{
    bool create = (argc > 1);

    try {
        Graph db("propertygraph", create ? Graph::Create : Graph::ReadOnly);

        Transaction tx(db, create ? Transaction::ReadWrite : 0);

        printf("Sizeof(time): %ld\n", sizeof(Time));
        struct tm tm_n, tm_e;
        struct tm tm_n1, tm_e1;
        struct tm tm_n2;
        int hr, min;
        // Test a time that will swich month/year when converted to UTC.
        string_to_tm("Wed Dec 31 16:59:04 PST 2014", &tm_n, &hr, &min);
        Time t_n(&tm_n, hr, min);  // PST diff
        // Test a time that falls in the daylight switch portion.
        string_to_tm("Sat Mar 8 17:59:24 PST 2014", &tm_n1, &hr, &min);
        Time t_n1(&tm_n1, hr, min);  // PST diff
        // Test the +/-HHMM format and leap year.
        string_to_tm("Tue Mar 1 04:00:00 +0530 2016", &tm_n2, &hr, &min);
        Time t_n2(&tm_n2, hr, min);  // Indian timezone
        // Test a simple time.
        string_to_tm("Wed Jul 30 16:59:04 EST 2014", &tm_e, &hr, &min);
        Time t_e(&tm_e, hr, min);  // EST diff
        // Test a time that falls in the daylight switch to the other side.
        string_to_tm("Sat Nov 1 18:59:24 PDT 2014", &tm_e1, &hr, &min);
        Time t_e1(&tm_e1, hr, min);  // EST diff

        printf("Testing UTC prints of all time variables:\n");
        printf("Given: %s, UTC: %s\n", time_to_string(t_n).c_str(),
                                       time_to_string(t_n, true).c_str());
        printf("Given: %s, UTC: %s\n", time_to_string(t_n1).c_str(),
                                       time_to_string(t_n1, true).c_str());
        printf("Given: %s, UTC: %s\n", time_to_string(t_n2).c_str(),
                                       time_to_string(t_n2, true).c_str());
        printf("Given: %s, UTC: %s\n", time_to_string(t_e).c_str(),
                                       time_to_string(t_e, true).c_str());
        printf("Given: %s, UTC: %s\n", time_to_string(t_e1).c_str(),
                                       time_to_string(t_e1, true).c_str());

        Node *prev = 0;
        for (int i = 1; i < argc; i++) {
            Node &n = db.add_node(0);
            n.set_property("id1", argv[i]);
            n.set_property("id2", i + 16ll);
            n.set_property("id3", -(1ull<<(i*4)));
            n.set_property("id4", "this is a very long string");
            n.set_property("id5", t_n);
            n.set_property("id6", t_n1);
            n.set_property("id7", t_n2);
            if (prev != NULL) {
                Edge &e = db.add_edge(*prev, n, 0);
                e.set_property("id3", prev->get_property("id1").string_value());
                e.set_property("id4", n.get_property("id1").string_value());
                e.set_property("id5", t_e);
                e.set_property("id6", t_e1);
            }
            prev = &n;
        }

        dump_nodes(db);
        dump_edges(db);
        tx.commit();
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
