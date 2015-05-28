/** Assumes an existing graph with some date property that can be
 *  specified as the second argument, since
 *  we do not provide a set_property function for date in the
 *  Java bindings API.
 *  Currently tested with propertygraph generated by propertytest
 *  using property name "id7" to test Indian time zone, count = 4 AND
 *  with emailindexgraph created using email.gson using property
 *  "DeliveryTime" and count = 13
 */
import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;
import jarvis.*;

public class DateTest {
    public static void main(String[] args)
    {
        if (args.length != 3) {
            System.out.println("Usage: DateTest <graph> <propertyname> <expected count>");
            System.out.println("Look for dates after July 1 2014 17:00 GMT.");
            System.exit(1);
        }
        String graph_name = args[0];
        String prop_id = args[1];
        int expected = Integer.parseInt(args[2]);

        // Zone abbreviations are not recommended. Use either full name
        // or best to give offset from GMT.
        // Somehow the abbreviation UTC does not work.
        Calendar c = Calendar.getInstance(TimeZone.getTimeZone("GMT-07:00")); // PDT
        c.set(Calendar.YEAR, 2014);
        c.set(Calendar.MONTH, 6);   // month number - 1
        c.set(Calendar.DATE, 1);
        c.set(Calendar.HOUR_OF_DAY, 10);  // Use HOUR for 12 hour time.
        c.set(Calendar.MINUTE, 0);
        c.set(Calendar.SECOND, 0);
        Date start = c.getTime();
        int count = 0;
        try {
            Graph db = new Graph(graph_name, Graph.OpenOptions.READONLY);
            Transaction tx = new Transaction(db, false, true);
            for (NodeIterator ni = db.get_nodes(); !ni.done(); ni.next()) {
                Node n = ni.get_current();
                Property np;
                if ( (np = n.get_property(prop_id)) != null && np.type() == Property.t_time) {
                    Date d = np.time_value().getDate();
                    if (d.after(start))
                        ++count;
                }
            }
            System.out.println("Count: " + count);
            if (count == expected)
                System.out.println("Test passed");
        } catch (jarvis.Exception e) {
            e.print();
            return;
        }
    }
}