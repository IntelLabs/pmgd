/*
 * This test checks Mantis #750
 */

#include <stdio.h>
#include "jarvis.h"
#include "../util/util.h"

using namespace Jarvis;

int main(int argc, char **argv)
{
    try {
        Graph db("test750graph", Graph::Create);

        switch (atoi(argv[1])) {
            case 1: {
                // Add nodes in first transaction
                Transaction tx(db, Transaction::ReadWrite);
                Node &n1 = db.add_node(0);
                n1.set_property("id", 1);
                Node &n2 = db.add_node(0);
                n2.set_property("id", 2);
                tx.commit();
                break;
            }

            case 2: {
                // Add property in second transaction
                Transaction tx(db, Transaction::ReadWrite);
                NodeIterator ni = db.get_nodes();
                Node &n1 = *ni;
                n1.set_property("a", 5);
                tx.commit();
                break;
            }

            case 3: {
                // Add property in third transaction
                // and then abort the transaction.
                Transaction tx(db, Transaction::ReadWrite);
                NodeIterator ni = db.get_nodes();
                Node &n1 = *ni;
                n1.set_property("abort", 2);
                exit(0);
            }

            case 4: {
                // Check that abort property is not present
                Transaction tx(db, Transaction::ReadOnly);
                NodeIterator ni = db.get_nodes();
                Node &n1 = *ni;
                printf("id = %lld\n", n1.get_property("id").int_value());
                Property p;
                if (n1.check_property("abort", p))
                    return 1;
                break;
            }
        }
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
