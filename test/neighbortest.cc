/*
 * This test checks Jarvis get_neighbors search iterator
 */

#include <stdio.h>
#include "jarvis.h"
#include "util.h"
#include "neighbor.h"

using namespace Jarvis;

int main(int argc, char **argv)
{
    bool create = (argc > 1);

    if (create) {
        try {
            Graph db("neighborgraph", Graph::Create);

            Transaction tx(db, Transaction::ReadWrite);

            Node &p = db.add_node("Person");
            p.set_property("Name", "John");
            Node &m = db.add_node("Message");
            m.set_property("UUID", "XYZZY");
            //Node &a = db.add_node("Attachment");
            //a.set_property("Path", "/x/y/z");
            db.add_edge(m, p, "To");
            //db.add_edge(m, a, "Attachment");

            tx.commit();
        }
        catch (Exception e) {
            print_exception(e);
            return 1;
        }
    }

    try {
        Graph db("neighborgraph");

        Transaction tx(db, Transaction::ReadWrite);

        /* In a graph with two connected nodes, my neighbor's neighbor
         * should be myself */
        NodeIterator pi = db.get_nodes("Person");
        NodeIterator mi = get_neighbors(*pi);
        if (&*get_neighbors(*mi) != &*pi) {
            fprintf(stderr, "neighbortest: failure\n");
            return 2;
        }
        if (mi->get_property("UUID") != "XYZZY") {
            fprintf(stderr, "neighbortest: failure\n");
            return 2;
        }

        db.remove(*pi->get_edges("To"));

        try {
            // This should throw.
            mi->get_property("UUID");
            fprintf(stderr, "neighbortest: vacant iterator failure\n");
            return 2;
        }
        catch (Exception e) {
            if (e.num != VacantIterator)
                throw;
        }

        // Don't commit the transaction, so the graph can be used again.
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }
}
