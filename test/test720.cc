/*
 * This test checks Mantis #720
 */

#include <stdio.h>
#include "jarvis.h"
#include "../util/util.h"

using namespace Jarvis;

int main(int argc, char **argv)
{
    int r = 0;

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
        NodeIterator ni1 = db.get_nodes();
        Node &nt1 = *ni1;
        printf("found node %p %s\n", &nt1, nt1.get_property(prop).string_value().c_str());
        if (nt1.get_property(prop).string_value() != val1)
            r = 1;
        if (ni1.next(),ni1) {
            printf("found node %p %s\n", &*ni1, ni1->get_property(prop).string_value().c_str());
            r = 1;
        }
        tx4.commit();

        // Remove a node and add a new one in third transaction
        // and then abort the transaction.
        {
        Transaction tx3(db, Transaction::ReadWrite);
        NodeIterator ni = db.get_nodes();
        Node &nt1 = *ni;
        printf("removing node %p %s\n", &nt1, nt1.get_property(prop).string_value().c_str());
        db.remove(nt1);

        Node &n2 = db.add_node(tag);
        n2.set_property(prop, val2);
        printf("added node %p %s\n", &n2, n2.get_property(prop).string_value().c_str());
        }

        // Check that node has its original value
        Transaction tx5(db, Transaction::ReadOnly);
        NodeIterator ni2 = db.get_nodes();
        Node &nt2 = *ni2;
        printf("found node %p %s\n", &nt2, nt2.get_property(prop).string_value().c_str());
        if (nt2.get_property(prop).string_value() != val1)
            r = 1;
        if (ni2.next(),ni2) {
            printf("found node %p %s\n", &*ni2, ni2->get_property(prop).string_value().c_str());
            r = 1;
        }
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return r;
}
