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
            Node &a = db.add_node("Attachment");
            a.set_property("Path", "/x/y/z");
            db.add_edge(m, p, "To");
            db.add_edge(m, a, "Attachment");

            tx.commit();
        }
        catch (Exception e) {
            print_exception(e);
            return 1;
        }
    }

    try {
        int r = 0;
        Graph db("neighborgraph");

        Transaction tx(db, Transaction::ReadWrite);

        NodeIterator pi = db.get_nodes("Person");
        NodeIterator mi = db.get_nodes("Message");
        NodeIterator ai = db.get_nodes("Attachment");

        /* The neighbor of each of my neighbors should be myself */
        printf("neighbor test 1\n");
        NodeIterator ni1 = get_neighbors(*mi);
        int n = 0;
        while (ni1) {
            n++;
            if (&*get_neighbors(*ni1) != &*mi) {
                fprintf(stderr, "neighbortest: failure 1a (%d)\n", n);
                r = 2;
            }
            ni1.next();
        }
        if (n != 2) {
            fprintf(stderr, "neighbortest: failure 1b (%d)\n", n);
            r = 2;
        }

        printf("neighbor test 2\n");
        std::vector<JointNeighborConstraint> v;
        v.push_back(JointNeighborConstraint{ Any, 0, *pi });
        v.push_back(JointNeighborConstraint{ Any, 0, *ai });
        NodeIterator ni2 = get_joint_neighbors(v);
        n = 0;
        while (ni2) {
            n++;
            if (&*ni2 != &*mi) {
                fprintf(stderr, "neighbortest: failure 2a (%d)\n", n);
                r = 2;
            }
            ni2.next();
        }
        if (n != 1) {
            fprintf(stderr, "neighbortest: failure 2b (%d)\n", n);
            r = 2;
        }


        NodeIterator ni3 = get_neighbors(*pi);

        if (ni3->get_property("UUID") != "XYZZY") {
            fprintf(stderr, "neighbortest: failure\n");
            r = 2;
        }

        db.remove(*pi->get_edges("To"));

        try {
            // This should throw.
            Property p = ni3->get_property("UUID");
            fprintf(stderr, "neighbortest: vacant iterator failure\n");
            dump(*ni3, stderr);
            r = 2;
        }
        catch (Exception e) {
            if (e.num != VacantIterator)
                throw;
        }

        // Don't commit the transaction, so the graph can be used again.
        return r;
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }
}
