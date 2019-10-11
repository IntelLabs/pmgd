/**
 * Find all emails to a user containing attachment of a particular
 * type received in a particular year.
 */

import pmgd.*;
import java.util.Vector;
import java.lang.String;
import java.lang.management.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.util.Date;

public class FindEmailWithAttachment {
    /**
     * Program name.
     */
    static String progName = "FindEmailWithAttachment";

    /*
     * Tags and properties. 
     */
    static String person_str = "Person";
    static String email_str = "Email";
    static String to_str = "To";
    static String attachment_str = "Attachment";
    static String filename_str = "Filename";
    static String deliverytime_str = "DeliveryTime";
    static String messageid_str = "MessageId";

    /**
     * The application's main function
     * @param argc the number of command line arguments
     * @param argv the array of command line arguments
     * @return status of execution (0 okay, 1 bad arguments, 2 processing error)
     */
    public static void main(String[] argv)
    {
        boolean timing = false;    ///< Enable timing measurement
        boolean verbose = false;
        int argc = argv.length;

        int num_emails;         ///< The number of emails

        // Parse the command line arguments
        int argi = 0;
        while (argi < argc && argv[argi].charAt(0) == '-') {
            if (argv[argi].compareTo("-h") == 0) {
                printUsage();
                return;
            }
            else if (argv[argi].compareTo("-t") == 0)
                timing = true;
            else if (argv[argi].compareTo("-v") == 0)
                verbose = true;
            else {
                System.out.println(progName + ": " + argv[argi] + ": Unrecognized option");
                return;
            }
            argi += 1;
        }
        if (argi + 1 > argc) {
            System.out.println(progName + ": No input graph");
            printUsage();
            return;
        }
        String graph_name = argv[argi];
        argi++;
        if (argi + 1 > argc) {
            System.out.println(progName + ": No recipient's email address");
            printUsage();
            return;
        }
        String recipient_email = argv[argi];
        argi++;
        if (argi + 1 > argc) {
            System.out.println(progName + ": No attachment type");
            printUsage();
            return;
        }
        String attachment_type = argv[argi]; // e.g., ".pptx" or ".pdf"
        argi++;
        if (argi + 1 > argc) {
            System.out.println(progName + ": No start of time range");
            printUsage();
            return;
        }

        Date start, end;
        SimpleDateFormat sdf = new SimpleDateFormat("EEE MMM dd HH:mm:ss zzz yyyy");
        try {
            start = sdf.parse(argv[argi]);
        } catch (ParseException pe) {
            System.out.println(progName + ": Unable to parse start date");
            return;
        }

        argi++;
        if (argi + 1 > argc) {
            System.out.println(progName + ": No end of time range");
            printUsage();
            return;
        }
        try {
            end = sdf.parse(argv[argi]);
        } catch (ParseException pe) {
            System.out.println(progName + ": Unable to parse end date");
            return;
        }
        ThreadMXBean bean = ManagementFactory.getThreadMXBean();
        long userTime, cpuTime;

        try {
            // Open the graph.
            Graph db = new Graph(graph_name, Graph.OpenOptions.ReadOnly);

            // Start a transaction.
            Transaction tx = new Transaction(db, false, true);

            // Convert C string to Strings.
            StringID person_sid = new StringID(person_str);
            StringID email_sid = new StringID(email_str);
            StringID to_sid = new StringID(to_str);
            StringID attachment_sid = new StringID(attachment_str);
            StringID filename_sid = new StringID(filename_str);
            StringID deliverytime_sid = new StringID(deliverytime_str);
            StringID messageid_sid = new StringID(messageid_str);

            // Initialize the number of recipients.
            num_emails = 0;
            Vector<Node> emails = new Vector<Node>();

            // Time measurement.
            long startTime = bean.getCurrentThreadUserTime();
            long startTotal = bean.getCurrentThreadCpuTime();

            try {
                // Get the node representing the recipient (we assume it is unique).
                NodeIterator ri = db.get_nodes(person_sid,
                        new PropertyPredicate(email_sid,
                            PropertyPredicate.Op.Eq,
                            new Property(recipient_email)), false);
                // If the recipient exists, look for messages.
                if (!ri.done()) {
                    for (NodeIterator mi = ri.get_current().get_neighbors(Node.Direction.Any, to_sid, false);
                            !mi.done();
                            mi.next()) {
                        for (NodeIterator ai = mi.get_current().get_neighbors(Node.Direction.Outgoing, attachment_sid, false);
                                !ai.done();
                                ai.next()) {
                            Property filenamep = ai.get_current().get_property(filename_sid);
                            if (filenamep != null) {
                                String s = filenamep.string_value();
                                Property deliverytimep;
                                if (s.endsWith(attachment_type) &&
                                    (deliverytimep = mi.get_current().get_property(deliverytime_sid)) != null) {
                                    Date t = deliverytimep.time_value().getDate();
                                    if (t.compareTo(start) >= 0 && t.compareTo(end) <= 0) {
                                        emails.add(mi.get_current());
                                        num_emails++;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch (pmgd.Exception e) {
                e.print();
                return;
            }

            // Time measurement.
            long endTime = bean.getCurrentThreadUserTime();
            long endTotal = bean.getCurrentThreadCpuTime();
            userTime = (endTime - startTime)/1000;
            cpuTime = (endTotal - startTotal)/1000;

            // Print a summary.
            System.out.println(num_emails);

            // Print timing info if requested.
            if (timing) {
                long sysTime = cpuTime - userTime;
                float upercent = 100 * (userTime / (float)cpuTime);
                float spercent = 100 * (sysTime / (float)cpuTime);
                System.out.println("Elapsed time is " + cpuTime + " microseconds");
                System.out.println("User time is " + userTime + " microseconds (" + upercent + "%)");
                System.out.println("System time is " + sysTime + " microseconds (" + spercent + "%)");
            }

            // Be more detailed if requested.
            if (verbose)
                for (Node n:emails)
                    System.out.println(n.get_property(messageid_sid).string_value());
        }
        catch (pmgd.Exception e) {
            e.print();
            return;
        }

        return;
    }

    /**
     * Print information about how to invoke this program
     * @param o the put-to stream
     */
    static void printUsage()
    {
        System.out.println("Usage: " + progName + " [OPTION]... GRAPH RECIPIENT TYPE START END");
        System.out.println("From GRAPH get a list of all emails sent between START and END to the given");
        System.out.println("RECIPIENT with the email containing at least one attachment of the given");
        System.out.println("TYPE.  TYPE is specified as a dotted suffix such as \".pdf\" or \".pptx\".");
        System.out.println("START and END must be Windows-compliant date specification such as");
        System.out.println("\"Wed Dec 31 06:25:48 PST 2003\".");
        System.out.println("");
        System.out.println("  -h  print this help and exit");
        System.out.println("  -t  time the execution");
        System.out.println("  -v  be verbose and print the messages' unique identifiers");
    }

}
