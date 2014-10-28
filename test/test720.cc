/*
 * This test checks Mantis #720
 */

#include <stdio.h>
#include "jarvis.h"
#include "../util/util.h"

using namespace Jarvis;

int main(int argc, char **argv)
{
    try {
        Graph db("test720graph", Graph::Create);

        static const char val1[] = "string value 1";
        static const char val2[] = "string value 2";

        // Add strings in first transaction
        Transaction tx1(db, Transaction::ReadWrite);
        StringID tag = "tag";
        StringID prop = "prop";
        tx1.commit();

        // Add node in second transaction
        Transaction tx2(db, Transaction::ReadWrite);
        Node &n1 = db.add_node(tag);
        n1.set_property(prop, val1);
        printf("added node %p %s\n", &n1, n1.get_property(prop).string_value().c_str());
        tx2.commit();

        // Change the node's property value in third transaction
        // and then abort the transaction.
        {
        Transaction tx3(db, Transaction::ReadWrite);
        NodeIterator ni = db.get_nodes();
        Node &n2 = *ni;
        n2.set_property(prop, val2);
        printf("changed property node %p %s\n", &n2, n2.get_property(prop).string_value().c_str());
        }

        // Check that node has its original value
        Transaction tx4(db, Transaction::ReadOnly);
        NodeIterator ni = db.get_nodes();
        Node &nt = *ni;
        printf("found node %p %s\n", &nt, nt.get_property(prop).string_value().c_str());
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
