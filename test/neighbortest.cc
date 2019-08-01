/**
 * @file   neighbortest.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

/*
 * This test checks PMGD get_neighbors search iterator
 */

#include <stdio.h>
#include <vector>
#include <algorithm>
#include "pmgd.h"
#include "util.h"
#include "neighbor.h"

using namespace PMGD;

template <typename T> int count(T &iter)
{
    int n;
    for (n = 0; iter; iter.next())
        n++;
    return n;
}

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

            Node &ann = db.add_node("Person");
            ann.set_property("Name", "Ann");
            Node &bob = db.add_node("Person");
            bob.set_property("Name", "Bob");
            Node &carl = db.add_node("Person");
            carl.set_property("Name", "Carl");
            Node &don = db.add_node("Person");
            don.set_property("Name", "Don");

            auto add_message =
                [&db](Node &from, const std::initializer_list<Node *> &to,
                      int order)
                {
                    static int next_id = 1;
                    Node &m = db.add_node("Message");
                    m.set_property("id", next_id++);
                    Edge &e = db.add_edge(from, m, "From");
                    e.set_property("order", order);
                    for (auto t : to)
                        db.add_edge(m, *t, "To");
                };

            add_message(ann, { &bob }, 0);
            add_message(bob, { &ann }, 1);
            add_message(ann, { &bob, &carl }, 2);
            add_message(bob, { &carl }, 3);
            add_message(bob, { &don }, 4);
            add_message(carl, { &bob, &don }, 5);
            add_message(bob, { &carl, &don }, 6);

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

        NodeIterator pi = db.get_nodes("Person", PropertyPredicate{ "Name", PropertyPredicate::Eq, "John" });
        NodeIterator mi = db.get_nodes("Message", PropertyPredicate{ "UUID", PropertyPredicate::Eq, "XYZZY" });
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


        printf("neighbor test 3\n");

        Node &ann = *db.get_nodes("Person",
                PropertyPredicate("Name", PropertyPredicate::Eq, "Ann"));
        Node &bob = *db.get_nodes("Person",
                PropertyPredicate("Name", PropertyPredicate::Eq, "Bob"));
        Node &carl = *db.get_nodes("Person",
                PropertyPredicate("Name", PropertyPredicate::Eq, "Carl"));
        Node &don = *db.get_nodes("Person",
                PropertyPredicate("Name", PropertyPredicate::Eq, "Don"));

        auto check_messages =
            [&db, &r](const std::vector<JointNeighborConstraint> &v,
                      std::vector<int> msgs)
            {
                static int test_id = 0;
                test_id++;
                NodeIterator ni = get_joint_neighbors(v);
                for (; ni; ni.next()) {
                    int id = ni->get_property("id").int_value();
                    auto pos = std::find(msgs.begin(), msgs.end(), id);
                    if (pos != msgs.end())
                        msgs.erase(pos);
                    else {
                        fprintf(stderr, "neighbortest: failure 3-%d(a) (%d)\n",
                                test_id, id);
                        r = 2;
                    }
                }
                if (!msgs.empty()) {
                    fprintf(stderr, "neighbortest: failure 3-%d(b):", test_id);
                    for (auto m : msgs)
                        fprintf(stderr, " %d", m);
                    fprintf(stderr, "\n");
                    r = 2;
                }
            };

        auto check_messages1 =
            [&db, &r, check_messages](Node &n1, Node &n2,
                                      const std::vector<int> &msgs)
            {
                check_messages(
                    { JointNeighborConstraint{ Any, 0, n1 },
                      JointNeighborConstraint{ Any, 0, n2 }  },
                    msgs);
            };

        auto check_messages2 =
            [&db, &r, check_messages](Node &from, Node &to,
                                      const std::vector<int> &msgs)
            {
                check_messages(
                    { JointNeighborConstraint{ Incoming, "From", from },
                      JointNeighborConstraint{ Outgoing, "To",   to   }  },
                    msgs);
            };

        try {
            check_messages({ }, { });
            fprintf(stderr, "neighbortest: expected range error exception from check_messages\n");
            r = 2;
        }
        catch (std::out_of_range) {
        }

        check_messages1(ann, bob, { 1, 2, 3 });
        check_messages2(ann, bob, { 1, 3 });
        check_messages2(ann, carl, { 3 });
        check_messages({ JointNeighborConstraint { Any, "To", ann } },
                       { 2 });
        check_messages({ JointNeighborConstraint { Incoming, "From", ann } },
                       { 1, 3 });
        check_messages({ JointNeighborConstraint { Outgoing, "To",   bob },
                         JointNeighborConstraint { Incoming, "From", ann } },
                       { 1, 3 });
        check_messages({ JointNeighborConstraint { Incoming, "From", bob } },
                       { 2, 4, 5, 7 });

        check_messages1(bob, carl, { 3, 4, 6, 7 });
        check_messages2(bob, carl, { 4, 7 });
        check_messages2(carl, bob, { 6 });
        check_messages({ JointNeighborConstraint { Any, "To", bob },
                         JointNeighborConstraint { Outgoing, "To", carl }  },
                       { 3 });

        check_messages1(bob, don, { 5, 6, 7 });
        check_messages2(bob, don, { 5, 7 });
        check_messages({ JointNeighborConstraint { Outgoing, "To", bob },
                         JointNeighborConstraint { Any, "To", don }  },
                       { 6 });

        check_messages({ JointNeighborConstraint { Incoming, "From", ann  },
                         JointNeighborConstraint { Any,      "To",   bob  },
                         JointNeighborConstraint { Outgoing, "To",   carl }  },
                       { 3 });

        check_messages({ JointNeighborConstraint { Any, "From", ann  },
                         JointNeighborConstraint { Any, "To",   bob  },
                         JointNeighborConstraint { Any, "To",   carl }  },
                       { 3 });

        check_messages({ JointNeighborConstraint { Outgoing, "To",   bob  },
                         JointNeighborConstraint { Incoming, "From", carl },
                         JointNeighborConstraint { Any,      "To",   don  }  },
                       { 6 });

        check_messages({ JointNeighborConstraint { Any, 0, carl },
                         JointNeighborConstraint { Any, 0, bob  },
                         JointNeighborConstraint { Any, 0, don  }  },
                       { 6, 7 });

        /* Nodes 1, 2, 3 are disconnected from the rest of the graph.
         * So a depth of 2 from Node 2 just returns its 1-hop neighbors
         * 1 and 3
         */
        printf("neighbor test 4\n");
        NodeIterator ni4 = get_neighborhood(*mi, 2, true);

        if (count(ni4) != 2) {
            fprintf(stderr, "neighbortest: failure 4 (%d)\n", n);
            r = 2;
        }

        NodeIterator ani = db.get_nodes("Person",
                PropertyPredicate("Name", PropertyPredicate::Eq, "Ann"));

        // There should be two neighbors at 2-hops of this node.
        printf("neighbor test 5a\n");
        NodeIterator ni5a = get_nhop_neighbors(*ani, 2);

        if (count(ni5a) != 2) {
            fprintf(stderr, "neighbortest: failure 5a (%d)\n", n);
            r = 2;
        }

        // Need 3-hops from Node 4 to cover most of the connected graph.
        // Test distance() computation.
        printf("neighbor test 5b\n");
        NeighborhoodIterator ni5b = get_neighborhood(*ani, 3, true);
        n = 0;
        int distance = 0;
        while (ni5b) {
            n++;
            distance = ni5b.distance();
            ni5b.next();
        }
        if (distance != 3) {
            fprintf(stderr, "neighbortest: failure 5b(1) (%d)\n", distance);
            r = 2;
        }
        if (n != 9) {
            fprintf(stderr, "neighbortest: failure 5b(2) (%d)\n", n);
            r = 2;
        }

        // This node is connected to pretty much the rest of the graph
        // except Nodes 1, 2, 3. So a depth of 2 gets to all the nodes
        // that can be reached from Node 5.
        NodeIterator bi = db.get_nodes("Person",
                PropertyPredicate("Name", PropertyPredicate::Eq, "Bob"));
        printf("neighbor test 6a\n");
        NodeIterator ni6a = get_neighborhood(*bi, 2, true);
        n = 0;
        while (ni6a) {
            n++;
            // Shouldn't return the starting node.
            if (&*ni6a == &*bi) {
                fprintf(stderr, "neighbortest: failure 6a(1) (%d)\n", n);
                r = 2;
            }
            ni6a.next();
        }
        if (n != 10) {
            fprintf(stderr, "neighbortest: failure 6a(2) (%d)\n", n);
            r = 2;
        }

        printf("neighbor test 6b\n");
        NeighborhoodIterator ni6b = get_neighborhood(*bi, 3, true);

        // Should return the same number of nodes as depth 2
        if (count(ni6b) != 10) {
            fprintf(stderr, "neighbortest: failure 6b (%d)\n", n);
            r = 2;
        }

        // Now follow only outgoing edges.
        printf("neighbor test 6c\n");
        NodeIterator ni6c = get_neighborhood(*bi, 2, Direction::Outgoing, true);

        if (count(ni6c) != 7) {
            fprintf(stderr, "neighbortest: failure 6c (%d)\n", n);
            r = 2;
        }

        // Now follow only outgoing edges with a tag.
        printf("neighbor test 6d\n");
        NeighborhoodIterator ni6d = get_neighborhood(*bi, 2, Direction::Outgoing, "From", true);

        if (count(ni6d) != 4) {
            fprintf(stderr, "neighbortest: failure 6d (%d)\n", n);
            r = 2;
        }

        // Now follow only outgoing edges with different tags.
        std::vector<EdgeConstraint> vn;
        vn.push_back(EdgeConstraint{ Outgoing, "From" });
        vn.push_back(EdgeConstraint{ Outgoing, "To" });
        printf("neighbor test 6e\n");
        NodeIterator ni6e = get_neighborhood(*bi, vn, true);

        if (count(ni6e) != 7) {
            fprintf(stderr, "neighbortest: failure 6e (%d)\n", n);
            r = 2;
        }


        printf("neighbor test 7\n");
        NodeIterator ni7 = get_neighbors(*pi);

        if (ni7->get_property("UUID") != "XYZZY") {
            fprintf(stderr, "neighbortest: failure\n");
            r = 2;
        }

        db.remove(*pi->get_edges("To"));

        try {
            // This should throw.
            Property p = ni7->get_property("UUID");
            fprintf(stderr, "neighbortest: vacant iterator failure\n");
            dump(*ni7, stderr);
            r = 2;
        }
        catch (Exception e) {
            if (e.num != VacantIterator)
                throw;
        }

        // Test to filter neighbors based on edge properties
        printf("neighbor test 8a\n");
        NodeIterator ann_node = db.get_nodes("Person",
                                             PropertyPredicate{ "Name",
                                                PropertyPredicate::Eq, "Ann" });

        NodeIterator ni8a = get_neighbors(*ann_node, Direction::Any, "From",
                                          { PropertyPredicate{ "order",
                                                PropertyPredicate::Ge, 1 }});

        if (count(ni8a) != 1) {
            fprintf(stderr, "neighbortest: failure 8a(%d)\n", n);
            r = 2;
        }

        printf("neighbor test 8b\n");

        NodeIterator ni8b = get_neighbors(*ann_node, Direction::Any, "From",
                                          { PropertyPredicate{ "order",
                                                PropertyPredicate::Eq, 0 }});

        if (count(ni8b) != 1) {
            fprintf(stderr, "neighbortest: failure 8b(%d)\n", n);
            r = 2;
        }

        printf("neighbor test 8c\n");

        NodeIterator bob_node = db.get_nodes("Person",
                                             PropertyPredicate{ "Name",
                                                PropertyPredicate::Eq, "Bob" });
        NodeIterator ni8c = get_neighbors(*bob_node, Direction::Any, "From",
                                          { PropertyPredicate{ "order",
                                                PropertyPredicate::Ge, 2 }});

        if (count(ni8c) != 3) {
            fprintf(stderr, "neighbortest: failure 8c(%d)\n", n);
            r = 2;
        }

        // Don't commit the transaction, so the graph can be used again.
        return r;
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }
}
