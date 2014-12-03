/*
 * This test checks Jarvis property lists
 */

#include "jarvis.h"
#include "util.h"
#include "annotate.h"

using namespace Jarvis;

int main(int argc, char **argv)
{
    const char *db_name = "propertylistgraph";
    int count = 1000;
    unsigned seed = unsigned(time(0));

    int argi = 1;

    if (argi < argc && !isdigit(argv[argi][0]))
        db_name = argv[argi++];
    if (argi < argc)
        count = atoi(argv[argi++]);
    if (argi < argc)
        seed = strtoull(argv[argi++], 0, 10);

    unsigned id_limit = count > 50000 ? 26*26 : 26;

    printf("count = %d, seed = %u\n", count, seed);

    srand(seed);

    try {
        Graph db(db_name, Graph::Create);

        annotate("Start transaction");
        Transaction tx(db);
        annotate("Add node");
        Node &n = db.add_node(0);
        annotate("Commit transaction");
        tx.commit();

        unsigned max_id = 0;
        for (int i = 0; i < count; i++) {
            unsigned id;
            if (rand() % id_limit < max_id)
                id = rand() % max_id;
            else
                id = max_id++;

            char id_str[] = { char('a' + id / 26), char('a' + id % 26), 0 };

            Property value;
            int type = rand() % 10;
            if (type < 1) {
                std::string s = std::string(id_str) + " D";
                printf("%s\n", s.c_str());
                annotate(s);
                Transaction tx(db);
                n.remove_property(id_str);
                tx.commit();
                continue;
            }
            if (type < 2) {
                // no value
            }
            else if (type < 3) {
                value = Property(bool(rand() % 1));
            }
            else if (type < 5) {
                int size = rand() % (rand() % 8 + 1) + 1;
                long long range = 1LL << (size * 8);
                value = Property((rand() % range) - range / 2);
            }
            else {
                int size = rand() % 17;
                value = Property("abcdefghijklmnopq", size);
            }

            std::string s = std::string(id_str) + " = " + property_text(value);
            printf("%s\n", s.c_str());
            annotate(s);
            Transaction tx(db);
            n.set_property(id_str, value);
            tx.commit();
        }
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
