/*
 * This test checks Jarvis property lists
 */

#include "jarvis.h"
#include "../util/util.h"

using namespace Jarvis;

int main(int argc, char **argv)
{
    try {
        Graph db("propertychunkgraph", Graph::Create);

        Transaction tx(db);

        Node &n1 = db.add_node(0);
        n1.set_property("id1", "14 char string");
        n1.set_property("id2", "14 char string");
        n1.set_property("id6", "this is a really long string");

        Node &n2 = db.add_node(0);
        n2.set_property("id1", "14 char string");
        n2.set_property("id2", "14 char string");
        n2.set_property("id3", true);
        n2.set_property("id4", true);
        n2.set_property("id5", true);
        n2.set_property("id6", true);

        Node &n3 = db.add_node(0);
        n3.set_property("id1", "14 char string");
        n3.set_property("id2", "14 char string");
        n3.set_property("id3", "8 char s");
        n3.set_property("id6", true);

        Node &n4 = db.add_node(0);
        n4.set_property("id1", "14 char string");
        n4.set_property("id2", "15  char string");
        n4.set_property("id3", "7 char s");
        n4.set_property("id6", true);

        Node &n5 = db.add_node(0);
        n5.set_property("id1", "14 char string");
        n5.set_property("id2", "14 char string");
        n5.set_property("id3", "short1");
        n5.set_property("id6", "short2");
        n5.set_property("id7", "this is another really long string");

        Node &n6 = db.add_node(0);
        n6.set_property("id1", "14 char string");
        n6.set_property("id3", "8 char s");
        n6.set_property("id2", "15  char string");
        n6.set_property("id6", true);

        dump_nodes(db);

        tx.commit();
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
