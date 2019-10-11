/*
 * Interface between the LDBC SNB driver and Jarvis Lake queries
 * Derived from Neo4j-based implementation of LDBC SNB by Alain Kaegi.
 * Copyright (C) 2016 Alain Kaegi
 * Copyright (C) 2016 Intel Corporation
 */

package jarvis.ldbc;

import java.io.IOException;

import java.util.Map;
import java.util.List;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.ListIterator;

import com.ldbc.driver.Db;
import com.ldbc.driver.DbConnectionState;
import com.ldbc.driver.DbException;
import com.ldbc.driver.OperationHandler;
import com.ldbc.driver.ResultReporter;

import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcNoResult;

import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery1;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery1Result;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery2;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery2Result;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery3;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery3Result;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery4;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery4Result;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery5;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery5Result;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery6;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery6Result;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery7;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery7Result;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery8;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery8Result;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery9;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery9Result;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery10;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery10Result;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery11;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery11Result;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery12;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery12Result;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery13;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery13Result;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery14;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcQuery14Result;

import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcShortQuery1PersonProfile;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcShortQuery1PersonProfileResult;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcShortQuery2PersonPosts;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcShortQuery2PersonPostsResult;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcShortQuery3PersonFriends;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcShortQuery3PersonFriendsResult;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcShortQuery4MessageContent;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcShortQuery4MessageContentResult;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcShortQuery5MessageCreator;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcShortQuery5MessageCreatorResult;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcShortQuery6MessageForum;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcShortQuery6MessageForumResult;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcShortQuery7MessageReplies;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcShortQuery7MessageRepliesResult;

import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcUpdate1AddPerson;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcUpdate2AddPostLike;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcUpdate3AddCommentLike;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcUpdate4AddForum;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcUpdate5AddForumMembership;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcUpdate6AddPost;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcUpdate7AddComment;
import com.ldbc.driver.workloads.ldbc.snb.interactive.LdbcUpdate8AddFriendship;

import pmgd.*;

public class JarvisDb extends Db {

    private class JarvisDbConnectionState extends DbConnectionState {

        private Graph db;

        public JarvisDbConnectionState(String db_name)
                throws pmgd.Exception
        {
            db = new Graph(db_name, Graph.OpenOptions.ReadWrite);
        }

        public Graph getDb() {
            return db;
        }

        @Override
        public void close() throws IOException {
        }
    }

    private class strings_t {
        public StringID Comment;
        public StringID EmailAddress;
        public StringID Forum;
        public StringID Language;
        public StringID Person;
        public StringID Place;
        public StringID Post;
        public StringID Tag;
        public StringID birthday;
        public StringID browserUsed;
        public StringID containerOf;
        public StringID content;
        public StringID creationDate;
        public StringID email;
        public StringID emailaddress;
        public StringID firstName;
        public StringID gender;
        public StringID hasCreator;
        public StringID hasMember;
        public StringID hasModerator;
        public StringID hasTag;
        public StringID id;
        public StringID imageFile;
        public StringID isLocatedIn;
        public StringID joinDate;
        public StringID knows;
        public StringID language;
        public StringID lastName;
        public StringID length;
        public StringID likes;
        public StringID locationIP;
        public StringID replyOf;
        public StringID speaks;
        public StringID title;

        strings_t(Graph db) throws pmgd.Exception
        {
            Transaction tx = new Transaction(db, false, true);

            Comment = new StringID("Comment");
            EmailAddress = new StringID("EmailAddress");
            Forum = new StringID("Forum");
            Language = new StringID("Language");
            Person = new StringID("Person");
            Place = new StringID("Place");
            Post = new StringID("Post");
            Tag = new StringID("Tag");
            birthday = new StringID("birthday");
            browserUsed = new StringID("browserUsed");
            containerOf = new StringID("containerOf");
            content = new StringID("content");
            creationDate = new StringID("creationDate");
            email = new StringID("email");
            emailaddress = new StringID("emailaddress");
            firstName = new StringID("firstName");
            gender = new StringID("gender");
            hasCreator = new StringID("hasCreator");
            hasMember = new StringID("hasMember");
            hasModerator = new StringID("hasModerator");
            hasTag = new StringID("hasTag");
            id = new StringID("id");
            imageFile = new StringID("imageFile");
            isLocatedIn = new StringID("isLocatedIn");
            joinDate = new StringID("joinDate");
            knows = new StringID("knows");
            language = new StringID("language");
            lastName = new StringID("lastName");
            length = new StringID("length");
            likes = new StringID("likes");
            locationIP = new StringID("locationIP");
            replyOf = new StringID("replyOf");
            speaks = new StringID("speaks");
            title = new StringID("title");

            tx.commit();
        }
    };

    private JarvisDbConnectionState dbConnectionState = null;
    private static strings_t strings;

    @Override
    protected void onInit(Map<String, String> properties) throws DbException {
        /* Register complex read queries. */
        registerOperationHandler(LdbcQuery1.class, Query1.class);
        registerOperationHandler(LdbcQuery2.class, Query2.class);
        registerOperationHandler(LdbcQuery3.class, Query3.class);
        registerOperationHandler(LdbcQuery4.class, Query4.class);
        registerOperationHandler(LdbcQuery5.class, Query5.class);
        registerOperationHandler(LdbcQuery6.class, Query6.class);
        registerOperationHandler(LdbcQuery7.class, Query7.class);
        registerOperationHandler(LdbcQuery8.class, Query8.class);
        registerOperationHandler(LdbcQuery9.class, Query9.class);
        registerOperationHandler(LdbcQuery10.class, Query10.class);
        registerOperationHandler(LdbcQuery11.class, Query11.class);
        registerOperationHandler(LdbcQuery12.class, Query12.class);
        registerOperationHandler(LdbcQuery13.class, Query13.class);
        registerOperationHandler(LdbcQuery14.class, Query14.class);

        /* Register short read queries. */
        registerOperationHandler(LdbcShortQuery1PersonProfile.class, ShortQuery1.class);
        registerOperationHandler(LdbcShortQuery2PersonPosts.class, ShortQuery2.class);
        registerOperationHandler(LdbcShortQuery3PersonFriends.class, ShortQuery3.class);
        registerOperationHandler(LdbcShortQuery4MessageContent.class, ShortQuery4.class);
        registerOperationHandler(LdbcShortQuery5MessageCreator.class, ShortQuery5.class);
        registerOperationHandler(LdbcShortQuery6MessageForum.class, ShortQuery6.class);
        registerOperationHandler(LdbcShortQuery7MessageReplies.class, ShortQuery7.class);

        /* Register update queries. */
        registerOperationHandler(LdbcUpdate1AddPerson.class, UpdateQuery1.class);
        registerOperationHandler(LdbcUpdate2AddPostLike.class, UpdateQuery2.class);
        registerOperationHandler(LdbcUpdate3AddCommentLike.class, UpdateQuery3.class);
        registerOperationHandler(LdbcUpdate4AddForum.class, UpdateQuery4.class);
        registerOperationHandler(LdbcUpdate5AddForumMembership.class, UpdateQuery5.class);
        registerOperationHandler(LdbcUpdate6AddPost.class, UpdateQuery6.class);
        registerOperationHandler(LdbcUpdate7AddComment.class, UpdateQuery7.class);
        registerOperationHandler(LdbcUpdate8AddFriendship.class, UpdateQuery8.class);

        String db_name = properties.get("url");
        try {
            dbConnectionState = new JarvisDbConnectionState(db_name);
            strings = new strings_t(dbConnectionState.getDb());
        }
        catch (pmgd.Exception e) {
            throw new DbException("Open "  + db_name + " failed");
        }
    }

    @Override
    protected void onClose() throws IOException {}

    @Override
    protected DbConnectionState getConnectionState() throws DbException {
        return dbConnectionState;
    }


    /* The complex read queries connectors. */

    public static class Query1 implements OperationHandler<LdbcQuery1, JarvisDbConnectionState> {
        @Override
        public void executeOperation(LdbcQuery1 operation,
                                     JarvisDbConnectionState dbConnectionState,
                                     ResultReporter result)
            throws DbException
        {
            try {
                Graph db = dbConnectionState.getDb();
                List<LdbcQuery1Result> r = query(db, operation.personId(), operation.firstName(), operation.limit());
                result.report(r.size(), r, operation);
            }
            catch (java.lang.Exception e) {
                System.out.print("Query1");
                List<LdbcQuery1Result> r = new ArrayList<LdbcQuery1Result>();
                result.report(r.size(), r, operation);
            }
        }
        private native List<LdbcQuery1Result> query(Graph db,
                long personId, String firstName, int limit);
    }

    public static class Query2 implements OperationHandler<LdbcQuery2, JarvisDbConnectionState> {
        @Override
        public void executeOperation(LdbcQuery2 operation, JarvisDbConnectionState dbConnectionState, ResultReporter result) throws DbException {
            try {
                Graph db = dbConnectionState.getDb();
                List<LdbcQuery2Result> r = query(db, operation.personId(), operation.maxDate().getTime(), operation.limit());
                result.report(r.size(), r, operation);
            }
            catch (java.lang.Exception e) {
                System.out.print("Query2");
                List<LdbcQuery2Result> r = new ArrayList<LdbcQuery2Result>();
                result.report(r.size(), r, operation);
            }
        }
        private native List<LdbcQuery2Result> query(Graph db,
                long personId, long date, int limit);
    }

    public static class Query3 implements OperationHandler<LdbcQuery3, JarvisDbConnectionState> {
        @Override
        public void executeOperation(LdbcQuery3 operation, JarvisDbConnectionState dbConnectionState, ResultReporter result) throws DbException {
            try {
                Graph db = dbConnectionState.getDb();
                List<LdbcQuery3Result> r = query(db, operation.personId(), operation.countryXName(), operation.countryYName(), operation.startDate().getTime(), operation.durationDays(), operation.limit());
                result.report(r.size(), r, operation);
            }
            catch (java.lang.Exception e) {
                System.out.print("Query3");
                List<LdbcQuery3Result> r = new ArrayList<LdbcQuery3Result>();
                result.report(r.size(), r, operation);
            }
        }
        private native List<LdbcQuery3Result> query(Graph db,
                long personId, String countryX, String countryY,
                long startDate, int duration, int limit);
    }

    public static class Query4 implements OperationHandler<LdbcQuery4, JarvisDbConnectionState> {
        @Override
        public void executeOperation(LdbcQuery4 operation, JarvisDbConnectionState dbConnectionState, ResultReporter result) throws DbException {
            try {
                Graph db = dbConnectionState.getDb();
                List<LdbcQuery4Result> r = query(db, operation.personId(), operation.startDate().getTime(), operation.durationDays(), operation.limit());
                result.report(r.size(), r, operation);
            }
            catch (java.lang.Exception e) {
                System.out.print("Query4");
                List<LdbcQuery4Result> r = new ArrayList<LdbcQuery4Result>();
                result.report(r.size(), r, operation);
            }
        }
        private native List<LdbcQuery4Result> query(Graph db,
                long personId, long startDate, int duration, int limit);
    }

    public static class Query5 implements OperationHandler<LdbcQuery5, JarvisDbConnectionState> {
        @Override
        public void executeOperation(LdbcQuery5 operation, JarvisDbConnectionState dbConnectionState, ResultReporter result) throws DbException {
            try {
                Graph db = dbConnectionState.getDb();
                List<LdbcQuery5Result> r = query(db, operation.personId(), operation.minDate().getTime(), operation.limit());
                result.report(r.size(), r, operation);
            }
            catch (java.lang.Exception e) {
                System.out.print("Query5");
                List<LdbcQuery5Result> r = new ArrayList<LdbcQuery5Result>();
                result.report(r.size(), r, operation);
            }
        }
        private native List<LdbcQuery5Result> query(Graph db,
                long personId, long date, int limit);
    }

    public static class Query6 implements OperationHandler<LdbcQuery6, JarvisDbConnectionState> {
        @Override
        public void executeOperation(LdbcQuery6 operation, JarvisDbConnectionState dbConnectionState, ResultReporter result) throws DbException {
            try {
                Graph db = dbConnectionState.getDb();
                List<LdbcQuery6Result> r = query(db, operation.personId(), operation.tagName(), operation.limit());
                result.report(r.size(), r, operation);
            }
            catch (java.lang.Exception e) {
                System.out.print("Query6");
                List<LdbcQuery6Result> r = new ArrayList<LdbcQuery6Result>();
                result.report(r.size(), r, operation);
            }
        }
        private native List<LdbcQuery6Result> query(Graph db,
                long personId, String tagName, int limit);
    }

    public static class Query7 implements OperationHandler<LdbcQuery7, JarvisDbConnectionState> {
        @Override
        public void executeOperation(LdbcQuery7 operation, JarvisDbConnectionState dbConnectionState, ResultReporter result) throws DbException {
            try {
                Graph db = dbConnectionState.getDb();
                List<LdbcQuery7Result> r = query(db, operation.personId(), operation.limit());
                result.report(r.size(), r, operation);
            }
            catch (java.lang.Exception e) {
                System.out.print("Query7");
                List<LdbcQuery7Result> r = new ArrayList<LdbcQuery7Result>();
                result.report(r.size(), r, operation);
            }
        }
        private native List<LdbcQuery7Result> query(Graph db,
                long personId, int limit);
    }

    public static class Query8 implements OperationHandler<LdbcQuery8, JarvisDbConnectionState> {
        @Override
        public void executeOperation(LdbcQuery8 operation, JarvisDbConnectionState dbConnectionState, ResultReporter result) throws DbException {
            try {
                Graph db = dbConnectionState.getDb();
                List<LdbcQuery8Result> r = query(db, operation.personId(), operation.limit());
                result.report(r.size(), r, operation);
            }
            catch (java.lang.Exception e) {
                System.out.print("Query8");
                List<LdbcQuery8Result> r = new ArrayList<LdbcQuery8Result>();
                result.report(r.size(), r, operation);
            }
        }
        private native List<LdbcQuery8Result> query(Graph db,
                long personId, int limit);
    }

    public static class Query9 implements OperationHandler<LdbcQuery9, JarvisDbConnectionState> {
        @Override
        public void executeOperation(LdbcQuery9 operation, JarvisDbConnectionState dbConnectionState, ResultReporter result) throws DbException {
            try {
                Graph db = dbConnectionState.getDb();
                List<LdbcQuery9Result> r = query(db, operation.personId(), operation.maxDate().getTime(), operation.limit());
                result.report(r.size(), r, operation);
            }
            catch (java.lang.Exception e) {
                System.out.print("Query9");
                List<LdbcQuery9Result> r = new ArrayList<LdbcQuery9Result>();
                result.report(r.size(), r, operation);
            }
        }
        private native List<LdbcQuery9Result> query(Graph db,
                long personId, long date, int limit);
    }

    public static class Query10 implements OperationHandler<LdbcQuery10, JarvisDbConnectionState> {
        @Override
        public void executeOperation(LdbcQuery10 operation, JarvisDbConnectionState dbConnectionState, ResultReporter result) throws DbException {
            try {
                Graph db = dbConnectionState.getDb();
                List<LdbcQuery10Result> r = query(db, operation.personId(), operation.month(), operation.limit());
                result.report(r.size(), r, operation);
            }
            catch (java.lang.Exception e) {
                System.out.print("Query10");
                List<LdbcQuery10Result> r = new ArrayList<LdbcQuery10Result>();
                result.report(r.size(), r, operation);
            }
        }
        private native List<LdbcQuery10Result> query(Graph db,
                long personId, int month, int limit);
    }

    public static class Query11 implements OperationHandler<LdbcQuery11, JarvisDbConnectionState> {
        @Override
        public void executeOperation(LdbcQuery11 operation, JarvisDbConnectionState dbConnectionState, ResultReporter result) throws DbException {
            try {
                Graph db = dbConnectionState.getDb();
                List<LdbcQuery11Result> r = query(db, operation.personId(), operation.countryName(), operation.workFromYear(), operation.limit());
                result.report(r.size(), r, operation);
            }
            catch (java.lang.Exception e) {
                System.out.print("Query11");
                List<LdbcQuery11Result> r = new ArrayList<LdbcQuery11Result>();
                result.report(r.size(), r, operation);
            }
        }
        private native List<LdbcQuery11Result> query(Graph db,
                long personId, String countryName, int year, int limit);
    }

    public static class Query12 implements OperationHandler<LdbcQuery12, JarvisDbConnectionState> {
        @Override
        public void executeOperation(LdbcQuery12 operation, JarvisDbConnectionState dbConnectionState, ResultReporter result) throws DbException {
            try {
                Graph db = dbConnectionState.getDb();
                List<LdbcQuery12Result> r = query(db, operation.personId(), operation.tagClassName(), operation.limit());
                result.report(r.size(), r, operation);
            }
            catch (java.lang.Exception e) {
                System.out.print("Query12");
                List<LdbcQuery12Result> r = new ArrayList<LdbcQuery12Result>();
                result.report(r.size(), r, operation);
            }
        }
        private native List<LdbcQuery12Result> query(Graph db,
                long personId, String tagClassName, int limit);
    }

    public static class Query13 implements OperationHandler<LdbcQuery13, JarvisDbConnectionState> {
        @Override
        public void executeOperation(LdbcQuery13 operation, JarvisDbConnectionState dbConnectionState, ResultReporter result) throws DbException {
            try {
                Graph db = dbConnectionState.getDb();
                LdbcQuery13Result r = query(db, operation.person1Id(), operation.person2Id());
                result.report(1, r, operation);
            }
            catch (java.lang.Exception e) {
                System.out.print("Query13");
                LdbcQuery13Result r = new LdbcQuery13Result(0);
                result.report(0, r, operation);
            }
        }
        private native LdbcQuery13Result query(Graph db,
                long person1Id, long person2Id);
    }

    public static class Query14 implements OperationHandler<LdbcQuery14, JarvisDbConnectionState> {
        @Override
        public void executeOperation(LdbcQuery14 operation, JarvisDbConnectionState dbConnectionState, ResultReporter result) throws DbException {
            try {
                Graph db = dbConnectionState.getDb();
                List<LdbcQuery14Result> r = query(db, operation.person1Id(), operation.person2Id());
                result.report(r.size(), r, operation);
            }
            catch (java.lang.Exception e) {
                System.out.print("Query14");
                List<LdbcQuery14Result> r = new ArrayList<LdbcQuery14Result>();
                result.report(r.size(), r, operation);
            }
        }
        private native List<LdbcQuery14Result> query(Graph db,
                long person1Id, long person2Id);
    }


    /* The short read queries connectors. */

    public static class ShortQuery1
            implements OperationHandler<LdbcShortQuery1PersonProfile,
                                        JarvisDbConnectionState>
    {
        @Override
        public void executeOperation(LdbcShortQuery1PersonProfile operation,
                                     JarvisDbConnectionState dbConnectionState,
                                     ResultReporter result)
            throws DbException
        {
            Graph db = dbConnectionState.getDb();
            Transaction tx = null;
            try {
                tx = new Transaction(db, false, true);
                Node person = db.get_node(strings.Person, strings.id, operation.personId());
                Node city = person.get_neighbor(Node.Direction.Outgoing,
                                                strings.isLocatedIn);
                LdbcShortQuery1PersonProfileResult r
                    = new LdbcShortQuery1PersonProfileResult(
                        person.get_property(strings.firstName).string_value(),
                        person.get_property(strings.lastName).string_value(),
                        convert_time(person.get_property(strings.birthday)),
                        person.get_property(strings.locationIP).string_value(),
                        person.get_property(strings.browserUsed).string_value(),
                        city.get_property(strings.id).int_value(),
                        person.get_property(strings.gender).string_value(),
                        convert_time(person.get_property(strings.creationDate)));

                tx.commit();
                result.report(1, r, operation);
            }
            catch (pmgd.Exception e) {
                tx.abort();
                System.out.print("ShortQuery1 ");
                e.print();
                LdbcShortQuery1PersonProfileResult r = new LdbcShortQuery1PersonProfileResult("", "", 0, "", "", 0, "", 0);
                result.report(0, r, operation);
            }
        }
    }

    public static class ShortQuery2
            implements OperationHandler<LdbcShortQuery2PersonPosts,
                                        JarvisDbConnectionState>
    {
        class record {
            public Node msg;
            public long id;
            public long date;
            public record(Node m, long i, long d) {
                msg = m;
                id = i;
                date = d;
            }
        };

        @Override
        public void executeOperation(LdbcShortQuery2PersonPosts operation,
                                     JarvisDbConnectionState dbConnectionState,
                                     ResultReporter result)
            throws DbException
        {
            Graph db = dbConnectionState.getDb();
            Transaction tx = null;
            try {
                int limit = operation.limit();
                tx = new Transaction(db, false, true);
                Node person = db.get_node(strings.Person, strings.id, operation.personId());
                NodeIterator msgs = person.get_neighbors
                                (Node.Direction.Incoming, strings.hasCreator, false);

                LinkedList<record> q = new LinkedList<record>();
                int count = 0;

                while (!msgs.done()) {
                    Node msg = msgs.get_current(); msgs.next();
                    long id = msg.get_property(strings.id).int_value();
                    long date = convert_time(msg.get_property(strings.creationDate));

                    if (count < limit || insert_before(q.getLast(), id, date))
                    {
                        ListIterator<record> i = q.listIterator();
                        while (i.hasNext()) {
                            record r = i.next();
                            if (insert_before(r, id, date)) {
                                i.previous();
                                break;
                            }
                        }

                        i.add(new record(msg, id, date));

                        if (count < limit)
                            count++;
                        else
                            q.removeLast();
                    }
                }

                List<LdbcShortQuery2PersonPostsResult> r = new ArrayList<LdbcShortQuery2PersonPostsResult>(count);
                for (record i : q) {
                    Node msg = i.msg;
                    Node post = get_post(msg);
                    Node creator = post.get_neighbor(Node.Direction.Outgoing,
                                                     strings.hasCreator);
                    r.add(new LdbcShortQuery2PersonPostsResult(
                        i.id,
                        get_content(msg),
                        convert_time(msg.get_property(strings.creationDate)),
                        post.get_property(strings.id).int_value(),
                        creator.get_property(strings.id).int_value(),
                        creator.get_property(strings.firstName).string_value(),
                        creator.get_property(strings.lastName).string_value()));
                }

                tx.commit();
                result.report(r.size(), r, operation);
            }
            catch (pmgd.Exception e) {
                tx.abort();
                System.out.print("ShortQuery2 ");
                e.print();
                result.report(0, new ArrayList<LdbcShortQuery2PersonPostsResult>(), operation);
            }
        }

        private boolean insert_before(record r, long id, long date)
        {
            return date > r.date || (date == r.date && id > r.id);
        }
    }

    public static class ShortQuery3
            implements OperationHandler<LdbcShortQuery3PersonFriends,
                                        JarvisDbConnectionState>
    {
        @Override
        public void executeOperation(LdbcShortQuery3PersonFriends operation,
                                     JarvisDbConnectionState dbConnectionState,
                                     ResultReporter result)
            throws DbException
        {
            Graph db = dbConnectionState.getDb();
            Transaction tx = null;
            try {
                tx = new Transaction(db, false, true);
                Node person = db.get_node(strings.Person, strings.id, operation.personId());
                EdgeIterator edges = person.get_edges(strings.knows);

                List<LdbcShortQuery3PersonFriendsResult> r = new LinkedList<LdbcShortQuery3PersonFriendsResult>();

                while (!edges.done()) {
                    Edge edge = edges.get_current(); edges.next();
                    Node tmp = edge.get_source();
                    Node friend = tmp.equals(person) ? edge.get_destination() : tmp;
                    long id = friend.get_property(strings.id).int_value();
                    long date = convert_time(edge.get_property(strings.creationDate));

                    ListIterator<LdbcShortQuery3PersonFriendsResult> i
                        = find_insertion_point(r, id, date);

                    // i is null if the person is already present,
                    // which can happen if there are redundant "knows"
                    // edges in the graph.
                    if (i != null) {
                        i.add(new LdbcShortQuery3PersonFriendsResult(
                            id,
                            friend.get_property(strings.firstName).string_value(),
                            friend.get_property(strings.lastName).string_value(),
                            date));
                    }
                }

                tx.commit();
                result.report(r.size(), r, operation);
            }
            catch (pmgd.Exception e) {
                tx.abort();
                System.out.print("ShortQuery3 ");
                e.print();
                result.report(0, new ArrayList<LdbcShortQuery3PersonFriendsResult>(), operation);
            }
        }

        private ListIterator<LdbcShortQuery3PersonFriendsResult>
            find_insertion_point(List<LdbcShortQuery3PersonFriendsResult> list,
                                 long id, long date)
        {
            ListIterator<LdbcShortQuery3PersonFriendsResult> i = list.listIterator();
            while (i.hasNext()) {
                LdbcShortQuery3PersonFriendsResult a = i.next();
                long a_date = a.friendshipCreationDate();
                long a_id = a.personId();

                if (date > a_date || (date == a_date && id <= a_id)) {
                    i.previous();
                    break;
                }

                if (id == a_id)
                    return null;
            }

            return i;
        }
    }

    public static class ShortQuery4
            implements OperationHandler<LdbcShortQuery4MessageContent,
                                        JarvisDbConnectionState>
    {
        @Override
        public void executeOperation(LdbcShortQuery4MessageContent operation,
                                     JarvisDbConnectionState dbConnectionState,
                                     ResultReporter result)
            throws DbException
        {
            Graph db = dbConnectionState.getDb();
            Transaction tx = null;
            try {
                tx = new Transaction(db, false, true);
                Node msg = get_message(db, operation.messageId());

                LdbcShortQuery4MessageContentResult r
                    = new LdbcShortQuery4MessageContentResult(
                        get_content(msg),
                        convert_time(msg.get_property(strings.creationDate)));

                tx.commit();
                result.report(1, r, operation);
            }
            catch (pmgd.Exception e) {
                tx.abort();
                System.out.print("ShortQuery4 ");
                e.print();
                LdbcShortQuery4MessageContentResult r = new LdbcShortQuery4MessageContentResult( "", 0);
                result.report(0, r, operation);
            }
        }
    }

    public static class ShortQuery5
            implements OperationHandler<LdbcShortQuery5MessageCreator,
                                        JarvisDbConnectionState>
    {
        @Override
        public void executeOperation(LdbcShortQuery5MessageCreator operation,
                                     JarvisDbConnectionState dbConnectionState,
                                     ResultReporter result)
            throws DbException
        {
            Graph db = dbConnectionState.getDb();
            Transaction tx = null;
            try {
                tx = new Transaction(db, false, true);
                Node msg = get_message(db, operation.messageId());
                Node creator = msg.get_neighbor(Node.Direction.Outgoing,
                                                strings.hasCreator);

                LdbcShortQuery5MessageCreatorResult r
                    = new LdbcShortQuery5MessageCreatorResult(
                        creator.get_property(strings.id).int_value(),
                        creator.get_property(strings.firstName).string_value(),
                        creator.get_property(strings.lastName).string_value());

                tx.commit();
                result.report(1, r, operation);
            }
            catch (pmgd.Exception e) {
                tx.abort();
                System.out.print("ShortQuery5 ");
                e.print();
                LdbcShortQuery5MessageCreatorResult r = new LdbcShortQuery5MessageCreatorResult( 0, "", "");
                result.report(0, r, operation);
            }
        }
    }

    public static class ShortQuery6
            implements OperationHandler<LdbcShortQuery6MessageForum,
                                        JarvisDbConnectionState>
    {
        @Override
        public void executeOperation(LdbcShortQuery6MessageForum operation,
                                     JarvisDbConnectionState dbConnectionState,
                                     ResultReporter result)
            throws DbException
        {
            Graph db = dbConnectionState.getDb();
            Transaction tx = null;
            try {
                tx = new Transaction(db, false, true);
                Node msg = get_message(db, operation.messageId());
                Node post = get_post(msg);
                Node forum = post.get_neighbor(Node.Direction.Incoming,
                                              strings.containerOf);
                Node mod = forum.get_neighbor(Node.Direction.Outgoing,
                                              strings.hasModerator);

                LdbcShortQuery6MessageForumResult r
                    = new LdbcShortQuery6MessageForumResult(
                        forum.get_property(strings.id).int_value(),
                        forum.get_property(strings.title).string_value(),
                        mod.get_property(strings.id).int_value(),
                        mod.get_property(strings.firstName).string_value(),
                        mod.get_property(strings.lastName).string_value());

                tx.commit();
                result.report(1, r, operation);
            }
            catch (pmgd.Exception e) {
                tx.abort();
                System.out.print("ShortQuery6 ");
                e.print();
                LdbcShortQuery6MessageForumResult r
                    = new LdbcShortQuery6MessageForumResult( 0, "", 0, "", "");
                result.report(0, r, operation);
            }
        }
    }

    public static class ShortQuery7
            implements OperationHandler<LdbcShortQuery7MessageReplies,
                                        JarvisDbConnectionState>
    {
        @Override
        public void executeOperation(LdbcShortQuery7MessageReplies operation,
                                     JarvisDbConnectionState dbConnectionState,
                                     ResultReporter result)
            throws DbException
        {
            Graph db = dbConnectionState.getDb();
            Transaction tx = null;
            try {
                tx = new Transaction(db, false, true);
                Node msg = get_message(db, operation.messageId());
                Node orig_author = msg.get_neighbor
                                (Node.Direction.Outgoing, strings.hasCreator);
                NodeIterator replies = msg.get_neighbors
                            (Node.Direction.Incoming, strings.replyOf, false);

                List<LdbcShortQuery7MessageRepliesResult> r
                    = new LinkedList<LdbcShortQuery7MessageRepliesResult>();

                while (!replies.done()) {
                    Node reply = replies.get_current(); replies.next();
                    long date = convert_time(reply.get_property(strings.creationDate));
                    Node author = reply.get_neighbor(Node.Direction.Outgoing,
                                                     strings.hasCreator);
                    long author_id = author.get_property(strings.id).int_value();

                    ListIterator<LdbcShortQuery7MessageRepliesResult> li = r.listIterator();
                    while (li.hasNext()) {
                        LdbcShortQuery7MessageRepliesResult a = li.next();
                        long a_date = a.commentCreationDate();
                        if (date > a_date || (date == a_date && author_id < a.replyAuthorId())) {
                            li.previous();
                            break;
                        }
                    }
                    li.add(new LdbcShortQuery7MessageRepliesResult(
                        reply.get_property(strings.id).int_value(),
                        reply.get_property(strings.content).string_value(),
                        date,
                        author_id,
                        author.get_property(strings.firstName).string_value(),
                        author.get_property(strings.lastName).string_value(),
                        knows(orig_author, author)));
                }

                tx.commit();
                result.report(r.size(), r, operation);
            }
            catch (pmgd.Exception e) {
                tx.abort();
                System.out.print("ShortQuery7 ");
                e.print();
                result.report(0, new ArrayList<LdbcShortQuery7MessageRepliesResult>(), operation);
            }
        }
    }


    /* The update queries connectors. */

    public static class UpdateQuery1
            implements OperationHandler<LdbcUpdate1AddPerson,
                                        JarvisDbConnectionState>
    {
        @Override
        public void executeOperation(LdbcUpdate1AddPerson operation,
                                     JarvisDbConnectionState dbConnectionState,
                                     ResultReporter result)
            throws DbException
        {
            Graph db = dbConnectionState.getDb();
            Transaction tx = null;
            try {
                tx = new Transaction(db, false, false);
                Node person = db.add_node(strings.Person);

                person.set_property(strings.id, new Property(operation.personId()));
                person.set_property(strings.firstName, new Property(operation.personFirstName()));
                person.set_property(strings.lastName, new Property(operation.personLastName()));
                person.set_property(strings.gender, new Property(operation.gender()));
                person.set_property(strings.birthday, new Property(operation.birthday()));
                person.set_property(strings.creationDate, new Property(operation.creationDate()));
                person.set_property(strings.locationIP, new Property(operation.locationIp()));
                person.set_property(strings.browserUsed, new Property(operation.browserUsed()));

                Node city = db.get_node(strings.Place, strings.id, operation.cityId());
                db.add_edge(person, city, strings.isLocatedIn);

                for (String s : operation.languages()) {
                    NodeIterator i = db.get_nodes(strings.Language, strings.language, s);
                    Node lang;
                    if (!i.done())
                        lang = i.get_current();
                    else  {
                        lang = db.add_node(strings.Language);
                        lang.set_property(strings.language, new Property(s));
                    }
                    db.add_edge(person, lang, strings.speaks);
                }

                // There is only one node with the tag EmailAddress.
                // The actual address is a property on the edge.
                NodeIterator i = db.get_nodes(strings.EmailAddress);
                Node n = !i.done() ? i.get_current()
                                   : db.add_node(strings.EmailAddress);
                for (String s : operation.emails()) {
                    Edge e = db.add_edge(person, n, strings.email);
                    e.set_property(strings.emailaddress, new Property(s));
                }

                tx.commit();
                result.report(0, LdbcNoResult.INSTANCE, operation);
            }
            catch (pmgd.Exception e) {
                tx.abort();
                System.out.print("UpdateQuery1 ");
                e.print();
                result.report(0, LdbcNoResult.INSTANCE, operation);
            }
        }
    }

    public static class UpdateQuery2
            implements OperationHandler<LdbcUpdate2AddPostLike,
                                        JarvisDbConnectionState>
    {
        @Override
        public void executeOperation(LdbcUpdate2AddPostLike operation,
                                     JarvisDbConnectionState dbConnectionState,
                                     ResultReporter result)
            throws DbException
        {
            Graph db = dbConnectionState.getDb();
            Transaction tx = null;
            try {
                tx = new Transaction(db, false, false);

                Node person = db.get_node(strings.Person, strings.id, operation.personId());
                Node post = db.get_node(strings.Post, strings.id, operation.postId());

                Edge e = db.add_edge(person, post, strings.likes);
                e.set_property(strings.creationDate, new Property(operation.creationDate()));

                tx.commit();
                result.report(0, LdbcNoResult.INSTANCE, operation);
            }
            catch (pmgd.Exception e) {
                tx.abort();
                System.out.print("UpdateQuery2 ");
                e.print();
                result.report(0, LdbcNoResult.INSTANCE, operation);
            }
        }
    }

    public static class UpdateQuery3
            implements OperationHandler<LdbcUpdate3AddCommentLike,
                                        JarvisDbConnectionState>
    {
        @Override
        public void executeOperation(LdbcUpdate3AddCommentLike operation,
                                     JarvisDbConnectionState dbConnectionState,
                                     ResultReporter result)
            throws DbException
        {
            Graph db = dbConnectionState.getDb();
            Transaction tx = null;
            try {
                tx = new Transaction(db, false, false);

                Node person = db.get_node(strings.Person, strings.id, operation.personId());
                Node comment = db.get_node(strings.Comment, strings.id, operation.commentId());

                Edge e = db.add_edge(person, comment, strings.likes);
                e.set_property(strings.creationDate, new Property(operation.creationDate()));

                tx.commit();
                result.report(0, LdbcNoResult.INSTANCE, operation);
            }
            catch (pmgd.Exception e) {
                tx.abort();
                System.out.print("UpdateQuery3 ");
                e.print();
                result.report(0, LdbcNoResult.INSTANCE, operation);
            }
        }
    }

    public static class UpdateQuery4
            implements OperationHandler<LdbcUpdate4AddForum,
                                        JarvisDbConnectionState>
    {
        @Override
        public void executeOperation(LdbcUpdate4AddForum operation,
                                     JarvisDbConnectionState dbConnectionState,
                                     ResultReporter result)
            throws DbException
        {
            Graph db = dbConnectionState.getDb();
            Transaction tx = null;
            try {
                tx = new Transaction(db, false, false);
                Node forum = db.add_node(strings.Forum);
                forum.set_property(strings.id, new Property(operation.forumId()));
                forum.set_property(strings.title, new Property(operation.forumTitle()));
                forum.set_property(strings.creationDate, new Property(operation.creationDate()));

                Node moderator = db.get_node(strings.Person, strings.id,
                                              operation.moderatorPersonId());
                db.add_edge(forum, moderator, strings.hasModerator);

                for (long tagid : operation.tagIds()) {
                    Node tag = db.get_node(strings.Tag, strings.id, tagid);
                    db.add_edge(forum, tag, strings.hasTag);
                }

                tx.commit();
                result.report(0, LdbcNoResult.INSTANCE, operation);
            }
            catch (pmgd.Exception e) {
                tx.abort();
                System.out.print("UpdateQuery4 ");
                e.print();
                result.report(0, LdbcNoResult.INSTANCE, operation);
            }
        }
    }

    public static class UpdateQuery5
            implements OperationHandler<LdbcUpdate5AddForumMembership,
                                        JarvisDbConnectionState>
    {
        @Override
        public void executeOperation(LdbcUpdate5AddForumMembership operation,
                                     JarvisDbConnectionState dbConnectionState,
                                     ResultReporter result)
            throws DbException
        {
            Graph db = dbConnectionState.getDb();
            Transaction tx = null;
            try {
                tx = new Transaction(db, false, false);

                Node person = db.get_node(strings.Person, strings.id, operation.personId());
                Node forum = db.get_node(strings.Forum, strings.id, operation.forumId());

                Edge e = db.add_edge(forum, person, strings.hasMember);
                e.set_property(strings.joinDate, new Property(operation.joinDate()));

                tx.commit();
                result.report(0, LdbcNoResult.INSTANCE, operation);
            }
            catch (pmgd.Exception e) {
                tx.abort();
                System.out.print("UpdateQuery5 ");
                e.print();
                result.report(0, LdbcNoResult.INSTANCE, operation);
            }
        }
    }

    public static class UpdateQuery6
            implements OperationHandler<LdbcUpdate6AddPost,
                                        JarvisDbConnectionState>
    {
        @Override
        public void executeOperation(LdbcUpdate6AddPost operation,
                                     JarvisDbConnectionState dbConnectionState,
                                     ResultReporter result)
            throws DbException
        {
            Graph db = dbConnectionState.getDb();
            Transaction tx = null;
            try {
                tx = new Transaction(db, false, false);
                Node author = db.get_node(strings.Person, strings.id, operation.authorPersonId());
                Node forum = db.get_node(strings.Forum, strings.id, operation.forumId());
                Node country = db.get_node(strings.Place, strings.id, operation.countryId());

                Node post = db.add_node(strings.Post);
                post.set_property(strings.id, new Property(operation.postId()));
                post.set_property(strings.imageFile, new Property(operation.imageFile()));
                post.set_property(strings.creationDate, new Property(operation.creationDate()));
                post.set_property(strings.locationIP, new Property(operation.locationIp()));
                post.set_property(strings.browserUsed, new Property(operation.browserUsed()));
                post.set_property(strings.language, new Property(operation.language()));
                post.set_property(strings.content, new Property(operation.content()));
                post.set_property(strings.length, new Property(operation.length()));
                db.add_edge(post, author, strings.hasCreator);
                db.add_edge(forum, post, strings.containerOf);
                db.add_edge(post, country, strings.isLocatedIn);

                for (long tagid : operation.tagIds()) {
                    Node tag = db.get_node(strings.Tag, strings.id, tagid);
                    db.add_edge(post, tag, strings.hasTag);
                }

                tx.commit();
                result.report(0, LdbcNoResult.INSTANCE, operation);
            }
            catch (pmgd.Exception e) {
                tx.abort();
                System.out.print("UpdateQuery6 ");
                e.print();
                result.report(0, LdbcNoResult.INSTANCE, operation);
            }
        }
    }

    public static class UpdateQuery7
            implements OperationHandler<LdbcUpdate7AddComment,
                                        JarvisDbConnectionState>
    {
        @Override
        public void executeOperation(LdbcUpdate7AddComment operation,
                                     JarvisDbConnectionState dbConnectionState,
                                     ResultReporter result)
            throws DbException
        {
            Graph db = dbConnectionState.getDb();
            Transaction tx = null;
            try {
                tx = new Transaction(db, false, false);
                Node author = db.get_node(strings.Person, strings.id, operation.authorPersonId());
                Node country = db.get_node(strings.Place, strings.id, operation.countryId());

                Node comment = db.add_node(strings.Comment);
                comment.set_property(strings.id, new Property(operation.commentId()));
                comment.set_property(strings.creationDate, new Property(operation.creationDate()));
                comment.set_property(strings.locationIP, new Property(operation.locationIp()));
                comment.set_property(strings.browserUsed, new Property(operation.browserUsed()));
                comment.set_property(strings.content, new Property(operation.content()));
                comment.set_property(strings.length, new Property(operation.length()));
                db.add_edge(comment, author, strings.hasCreator);
                db.add_edge(comment, country, strings.isLocatedIn);

                long post_id = operation.replyToPostId();
                long comment_id = operation.replyToCommentId();
                Node msg = post_id != -1
                               ? db.get_node(strings.Post, strings.id, post_id)
                               : db.get_node(strings.Comment, strings.id, comment_id);
                db.add_edge(comment, msg, strings.replyOf);

                for (long tagid : operation.tagIds()) {
                    Node tag = db.get_node(strings.Tag, strings.id, tagid);
                    db.add_edge(comment, tag, strings.hasTag);
                }

                tx.commit();
                result.report(0, LdbcNoResult.INSTANCE, operation);
            }
            catch (pmgd.Exception e) {
                tx.abort();
                System.out.print("UpdateQuery7 ");
                e.print();
                result.report(0, LdbcNoResult.INSTANCE, operation);
            }
        }
    }

    public static class UpdateQuery8
            implements OperationHandler<LdbcUpdate8AddFriendship,
                                        JarvisDbConnectionState>
    {
        @Override
        public void executeOperation(LdbcUpdate8AddFriendship operation,
                                     JarvisDbConnectionState dbConnectionState,
                                     ResultReporter result)
            throws DbException
        {
            Graph db = dbConnectionState.getDb();
            Transaction tx = null;
            try {
                tx = new Transaction(db, false, false);
                Node p1 = db.get_node(strings.Person, strings.id, operation.person1Id());
                Node p2 = db.get_node(strings.Person, strings.id, operation.person2Id());
                Edge e = db.add_edge(p1, p2, strings.knows);
                e.set_property(strings.creationDate, new Property(operation.creationDate()));
                tx.commit();
                result.report(0, LdbcNoResult.INSTANCE, operation);
            }
            catch (pmgd.Exception e) {
                tx.abort();
                System.out.print("UpdateQuery8 ");
                e.print();
                result.report(0, LdbcNoResult.INSTANCE, operation);
            }
        }
    }

    static private Node get_message(Graph db, long messageId)
        throws pmgd.Exception
    {
        PropertyPredicate pp = new PropertyPredicate(strings.id, messageId);
        NodeIterator i = db.get_nodes(strings.Post, pp);
        if (!i.done())
            return i.get_current();
        return db.get_node(strings.Comment, pp);
    }

    static private Node get_post(Node msg)
        throws pmgd.Exception
    {
        Node m = msg;
        NodeIterator i;
        while (!(i = m.get_neighbors(Node.Direction.Outgoing, strings.replyOf, false)).done())
            m = i.get_current();
        return m;
    }

    static private boolean knows(Node p1, Node p2)
        throws pmgd.Exception
    {
        if (p1.equals(p2))
            return false;

        NodeIterator i = p1.get_neighbors(strings.knows, false);
        while (!i.done()) {
            if (i.get_current().equals(p2))
                return true;
            i.next();
        }

        return false;
    }

    static private String get_content(Node msg)
        throws pmgd.Exception
    {
        Property content = msg.get_property(strings.content);
        if (content != null) {
            String val = content.string_value();
            if (!val.isEmpty())
                return val;
        }
        return msg.get_property(strings.imageFile).string_value();
    }

    static long convert_time(Property time)
        throws pmgd.Exception
    {
        Property.Time t = time.time_value();
        return t.getDate().getTime();
    }

    static {
        System.loadLibrary("pmgd-jni");
        System.loadLibrary("queries-jni");
    }
}
