public class BindingsTest{
    public static void main(String[] args){
	String sample_loc = args.length > 0 ? args[0] : "bindingsgraph";

        try {
	Graph db = new Graph(sample_loc, Graph.OpenOptions.NONE);

	//Adding Node, getting ID, tag
	Transaction tx1 = new Transaction(db, true, false);
	Node n1 = db.add_node("myTag1");
	int rc = db.get_id(n1);
	String ret = n1.get_tag();
	tx1.commit();
	System.out.printf("Adding node returned id %d, tag %s.\n", 
			  rc, ret);

	//Adding Edge, getting ID, tag
	Transaction tx2 = new Transaction(db, false, false);
	Node n2 = db.add_node("myTag2");
	Edge e1 = db.add_edge(n1, n2, "myTag3");
	rc  = db.get_id(e1);
	ret = e1.get_tag();
	tx2.commit();
	System.out.printf("Adding edge returned id %d, tag %s.\n", 
			  rc, ret);


	//Checking source and destination on added edge
	Transaction tx3 = new Transaction(db, false, true);
	Node src = e1.get_source();
	Node dest = e1.get_destination();
	int src_id = db.get_id(src);
	int dest_id = db.get_id(dest);
	tx3.commit();
	System.out.printf("Verified: edge goes from %d to %d.\n",
			  src_id, dest_id);

	//create some properties
	int v1 = 3;
	double v2 = 0.7202013;
	    
	Property p = new Property();

	Property p1 = new Property(p);
	Property p2 = new Property(true);
	Property p3 = new Property(v1);
	Property p4 = new Property("I am a long string property");
	Property p5 = new Property(v2);

	//Check types and get values of properties we just created
	System.out.printf("Checking type: %d\n", p.type());
	System.out.printf("Checking type: %d\n", p1.type());
	System.out.printf("Checking type: %d\n", p2.type());
	System.out.printf("Checking type: %d\n", p3.type());
	System.out.printf("Checking type: %d\n", p4.type());
	System.out.printf("Checking type: %d\n", p5.type());

	System.out.printf("The bool value of this is: %b.\n",
			  p2.bool_value());
	System.out.printf("The int value of this is: %d.\n",
			  p3.int_value());
	System.out.printf("The string value of this is: %s.\n",
			  p4.string_value());

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

	Node n4 = db.add_node("myTag2");
	n4.set_property("Name", new Property("vishakha"));


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


	// Do getnodes and verify by IDs that we are gettign everything
	Transaction tx6 = new Transaction(db, false, true);
	NodeIterator ni = db.get_nodes();
	for (int i = 1; !ni.done(); ni.next()) {
	    rc = db.get_id(ni.get_current());
            System.out.printf("Node iterator(%d), id %d #%s:\n",
                              i, rc, ni.get_tag());
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
            System.out.printf("Edge iterator(%d), id %d #%s:\n",
                              i, rc, ei.get_tag());
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

	tx6.abort();

	//Dump it out for verification purposes
	Transaction tx = new Transaction(db, false, true);
	db.dumpGraph();
	tx.commit();
        }catch(Exception e) {
            e.print();
            return;
        }
    }

}
