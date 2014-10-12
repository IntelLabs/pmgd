public class BindingsTest {
    public static void main(String[] args)
    {
        String sample_loc = args.length > 0 ? args[0] : "bindingsgraph";

        try {
            Graph db = new Graph(sample_loc, Graph.OpenOptions.NONE);

            // Adding Node, getting ID, tag
            Transaction tx1 = new Transaction(db, true, false);
            Node n1 = db.add_node("myTag1");
            int rc = db.get_id(n1);
            String ret = n1.get_tag();
            tx1.commit();
            System.out.printf("Adding node returned id %d, tag %s.\n", rc, ret);

            // Adding Edge, getting ID, tag
            Transaction tx2 = new Transaction(db, false, false);
            Node n2 = db.add_node("myTag2");
            Edge e1 = db.add_edge(n1, n2, "myTag3");
            rc  = db.get_id(e1);
            ret = e1.get_tag();
            tx2.commit();
            System.out.printf("Adding edge returned id %d, tag %s.\n", rc, ret);


            // Checking source and destination on added edge
            Transaction tx3 = new Transaction(db, false, true);
            Node src = e1.get_source();
            Node dest = e1.get_destination();
            int src_id = db.get_id(src);
            int dest_id = db.get_id(dest);
            tx3.commit();
            System.out.printf("Verified: edge goes from %d to %d.\n", src_id, dest_id);

            // Create some properties
            int v1 = 3;
            double v2 = 0.7202013;

            Property p = new Property();

            Property p1 = new Property(p);
            Property p2 = new Property(true);
            Property p3 = new Property(v1);
            Property p4 = new Property("I am a long string property");
            Property p5 = new Property(v2);

            // Check types and get values of properties we just created
            System.out.printf("Checking type: %d\n", p.type());
            System.out.printf("Checking type: %d\n", p1.type());
            System.out.printf("Checking type: %d\n", p2.type());
            System.out.printf("Checking type: %d\n", p3.type());
            System.out.printf("Checking type: %d\n", p4.type());
            System.out.printf("Checking type: %d\n", p5.type());

            System.out.printf("The bool value of this is: %b.\n", p2.bool_value());
            System.out.printf("The int value of this is: %d.\n", p3.int_value());
            System.out.printf("The string value of this is: %s.\n", p4.string_value());

            // Add, check, get, and remove properties from new nodes
            Transaction tx4 = new Transaction(db, false, false);
            Property p_name = new Property("katelin");
            Property p_age = new Property(26);
            n1.set_property("Name", p_name);
            n1.set_property("Age", p_age);

            n2.set_property("Name", new Property("philip"));
            n2.set_property("Age", new Property(52));

            Node n3 = db.add_node("myTag2");
            n3.set_property("Name", new Property("alain"));

            Node n4 = db.add_node(null);
            n4.set_property("Name", new Property("vishakha"));

            Edge e2 = db.add_edge(n1, n3, "myTag3");
            Edge e3 = db.add_edge(n4, n1, null);
            Edge e4 = db.add_edge(n3, n2, null);
            Edge e5 = db.add_edge(n4, n2, "myTag3");

            Property p6 = n1.get_property("Name");
            if (p6 != null)
                System.out.printf(" get returns:%s\n", p6.string_value());
            n1.remove_property("Age");
            Property p7 = n1.get_property("Age");
            if (p7 != null)
                System.out.printf("get returns:%s\n", p7.int_value());
            tx4.commit();

            // Add, check, get, and remove properties from new edge 1
            Transaction tx5 = new Transaction(db, false, false);
            Property p_relation = new Property("creates");
            Property p_count = new Property(-1);
            e1.set_property("Relation", p_relation);
            e1.set_property("Count", p_count);


            Property p8 = e1.get_property("Relation");
            if (p8 != null)
                System.out.printf(" get returns:%s\n", p8.string_value());
            e1.remove_property("Count");
            Property p9 = n1.get_property("Count");
            if (p9 != null)
                System.out.printf("get returns:%s\n", p9.int_value());
            tx5.commit();


            // Do get_nodes and verify by IDs that we are getting everything
            Transaction tx6 = new Transaction(db, false, true);
            NodeIterator ni = db.get_nodes();
            for (int i = 1; !ni.done(); ni.next()) {
                rc = db.get_id(ni.get_current());
                System.out.printf("Node iterator(%d), id %d%s:\n",
                    i, rc, tag_text(ni.get_tag()));
                for (PropertyIterator pi = ni.get_properties(); !pi.done(); pi.next()) {
                    System.out.printf("  %s [%d] ", pi.id(), pi.type());
                    switch (pi.type()) {
                        case Property.t_novalue:
                            break;
                        case Property.t_boolean:
                            System.out.printf("%s", pi.bool_value() ? "T" : "F");
                            break;
                        case Property.t_integer:
                            System.out.printf("%d", pi.int_value());
                            break;
                        case Property.t_string:
                            System.out.printf("%s", pi.string_value());
                            break;
                        case Property.t_float:
                            System.out.printf("%f", pi.float_value());
                            break;
                        default:
                            System.out.printf("?");
                            break;
                    }
                    System.out.printf("\n");
                }
                i++;
            }
            EdgeIterator ei = db.get_edges();
            for (int i = 1; !ei.done(); ei.next()) {
                rc = db.get_id(ei.get_current());
                System.out.printf("Edge iterator(%d), id %d%s:\n",
                    i, rc, tag_text(ei.get_tag()));
                for (PropertyIterator pi = ei.get_properties(); !pi.done(); pi.next()) {
                    System.out.printf("  %s [%d] ", pi.id(), pi.type());
                    switch (pi.type()) {
                        case Property.t_novalue:
                            break;
                        case Property.t_boolean:
                            System.out.printf("%s", pi.bool_value() ? "T" : "F");
                            break;
                        case Property.t_integer:
                            System.out.printf("%d", pi.int_value());
                            break;
                        case Property.t_string:
                            System.out.printf("%s", pi.string_value());
                            break;
                        case Property.t_float:
                            System.out.printf("%f", pi.float_value());
                            break;
                        default:
                            System.out.printf("?");
                            break;
                    }
                    System.out.printf("\n");
                }
                i++;
            }

            ni = db.get_nodes(null, new PropertyPredicate("Name"), false);
            for ( ; !ni.done(); ni.next())
                System.out.printf("Node %d has Name\n", db.get_id(ni.get_current()));

            ni = db.get_nodes(null, new PropertyPredicate("Age"), false);
            for ( ; !ni.done(); ni.next())
                System.out.printf("Node %d has Age\n", db.get_id(ni.get_current()));

            System.out.printf("Nodes with name > m\n");
            ni = db.get_nodes(null, new PropertyPredicate("Name", PropertyPredicate.op_t.ge, new Property("m")), false);
            for ( ; !ni.done(); ni.next())
                System.out.printf("Node %d: %s\n",
                    db.get_id(ni.get_current()),
                    ni.get_property("Name").string_value());

            System.out.printf("Nodes with name > f and name < s\n");
            ni = db.get_nodes(null, new PropertyPredicate("Name", PropertyPredicate.op_t.gtlt, new Property("f"), new Property("s")), false);
            for ( ; !ni.done(); ni.next())
                System.out.printf("Node %d: %s\n",
                    db.get_id(ni.get_current()),
                    ni.get_property("Name").string_value());

            System.out.printf("Nodes with tag myTag2 and name > a and name < s\n");
            ni = db.get_nodes("myTag2", new PropertyPredicate("Name", PropertyPredicate.op_t.gtlt, new Property("a"), new Property("s")), false);
            for ( ; !ni.done(); ni.next())
                System.out.printf("Node %d: %s\n",
                    db.get_id(ni.get_current()),
                    ni.get_property("Name").string_value());

            System.out.printf("All edges to/from node 1:");
            for (ei = n1.get_edges(); !ei.done(); ei.next())
                System.out.printf(" %d", db.get_id(ei.get_current()));
            System.out.printf("\n");

            System.out.printf("All edges to/from node 1:");
            ei = n1.get_edges(Node.Direction.ANY);
            for ( ; !ei.done(); ei.next())
                System.out.printf(" %d", db.get_id(ei.get_current()));
            System.out.printf("\n");

            System.out.printf("Edges to/from node 1 with tag:");
            ei = n1.get_edges("myTag3");
            for ( ; !ei.done(); ei.next())
                System.out.printf(" %d", db.get_id(ei.get_current()));
            System.out.printf("\n");

            System.out.printf("Edges to/from node 1 with tag:");
            ei = n1.get_edges(Node.Direction.ANY, "myTag3");
            for ( ; !ei.done(); ei.next())
                System.out.printf(" %d", db.get_id(ei.get_current()));
            System.out.printf("\n");

            System.out.printf("Edges from node 1 with tag:");
            ei = n1.get_edges(Node.Direction.OUTGOING, "myTag3");
            for ( ; !ei.done(); ei.next())
                System.out.printf(" %d", db.get_id(ei.get_current()));
            System.out.printf("\n");

            System.out.printf("Edges to node 2 with tag:");
            ei = n2.get_edges(Node.Direction.INCOMING, "myTag3");
            for ( ; !ei.done(); ei.next())
                System.out.printf(" %d", db.get_id(ei.get_current()));
            System.out.printf("\n");

            tx6.abort();

            // Dump it out for verification purposes
            dump(db);

        } catch (Exception e) {
            e.print();
            return;
        }
    }

    static void dump(Graph db) throws Exception
    {
        Transaction tx = new Transaction(db, false, true);
        for (NodeIterator i = db.get_nodes(); !i.done(); i.next())
            dump(db, i.get_current());
        for (EdgeIterator i = db.get_edges(); !i.done(); i.next())
            dump(db, i.get_current());
        tx.commit();
    }

    static void dump(Graph db, Node n) throws Exception
    {
        System.out.printf("Node %d%s:\n", db.get_id(n), tag_text(n.get_tag()));
        for (PropertyIterator i = n.get_properties(); !i.done(); i.next())
            System.out.printf("  %s: %s\n", i.id(), property_text(i.get_current()));

        for (EdgeIterator i = n.get_edges(Node.Direction.OUTGOING); !i.done(); i.next())
            System.out.printf(" %s -> n%d (e%d)\n", tag_text(i.get_tag()),
                              db.get_id(i.get_destination()),
                              db.get_id(i.get_current()));

        for (EdgeIterator i = n.get_edges(Node.Direction.INCOMING); !i.done(); i.next())
            System.out.printf(" %s <- n%d (e%d)\n", tag_text(i.get_tag()),
                              db.get_id(i.get_source()),
                              db.get_id(i.get_current()));
    }

    static void dump(Graph db, Edge e) throws Exception
    {
        System.out.printf("Edge %d%s: n%d -> n%d\n",
            db.get_id(e), tag_text(e.get_tag()),
            db.get_id(e.get_source()), db.get_id(e.get_destination()));

        for (PropertyIterator i = e.get_properties(); !i.done(); i.next())
            System.out.printf("  %s: %s\n", i.id(), property_text(i.get_current()));
    }

    static String tag_text(String tag)
    {
        if (tag != null)
            return " #" + tag;
        else
            return "";
    }

    static String property_text(Property p) throws Exception
    {
        switch (p.type()) {
            case Property.t_novalue: return "no value";
            case Property.t_boolean: return p.bool_value() ? "T" : "F";
            case Property.t_integer: return Long.toString(p.int_value());
            case Property.t_string: return p.string_value();
            case Property.t_float: return Double.toString(p.float_value());
            case Property.t_time: return "<time value>";
            case Property.t_blob: return "<blob value>";
            default: return "<unknown property type>";
        }
    }
}
