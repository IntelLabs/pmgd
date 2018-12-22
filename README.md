# Persistent Memory Graph Database (PMGD)

Recent developments in persistent memory technologies like 3D XPoint
promise storage elements  providing  nearly  the  speed  of  DRAM  and  the
durability of block-oriented storage. To provide an efficient storage
solution addressing the increasing popularity of connected data and
applications that benefit from graph like processing, we have designed
and implemented an in-persistent-memory graph database, PMGD, optimized
to run on a platform equipped with a vast amount of persistent memory.

## Features

* **Atomicity, consistency, isolation, and durability (ACID)**: As expected,
each transaction either completes entirely or not at all,
the database remains consistent, and a completed transaction’s
effects survive any failure. Concurrency is supported since version 2.0.0.

* **Recovery or restart from PM**: Should the system fail, PMGD
recovers  to  a  previous  consistent  state  upon  restart,  without
requiring its content to be loaded into memory. Thus, it avoids
any access to disk or unmarshalling of data from a disk-friendly
format  to  an  application  usable  format,  unlike  other  existing
disk-based graph databases.

* **Java bindings**: PMGD is a C++ implementation with client
bindings available for Java. This allows us to observe some
performance differences due to our use of C++.

In the current release, we  have  focused  our  efforts  on  understanding
challenges presented by a PM-based design before considering
a case for scaling the database out. Since the current expectations
for persistent memory offer the prospect of individual
platforms with memory capacity in terabytes, we can still evaluate
graph sizes that cover a large spectrum of deployments without
scaling  out.  Hence,  PMGD  currently  supports  a  single  node
operation. It is implemented as a library that is linked into an application
and as a server that can be accessed by multiple clients. We
plan to work on a distributed solution soon.

## System Overview

Graphs stored in PMGD consist of nodes (or vertices) optionally
connected  with  edges  (or  relationships).
Graphs  may  be  directed or undirected.
PMGD always stores directed edges but
its interface is such that direction may be ignored. All nodes need
not be connected; a directed graph may be weakly connected (i.e., a
path may not exist between all pairs of nodes).

PMGD supports
a property graph model with the following features:

* Each node and each edge has an associated tag that can be used
for classification. A tag is a short string that groups items into
classes. For example, in a metadata graph of emails,
a tag may be "Person", "Message",
"To", or "Keyword". In applications that don’t require
tags, they may be omitted.

* Each node and each edge may have associated properties (dis-
tinct from tags) stored as key-value pairs. Keys are short strings.
Values  must  be  one  of  six  predefined  types:  booleans,  integers,
floats,  strings,  date-time,  or  blobs  (i.e.,  arbitrary  strings
of bits). With the exception of blobs, all other predefined types
are  orderable.  Examples  of  properties  include
<"Firstname", "Leonhard">, <"Lastname", "Euler">, and so on.

These features provide a powerful data model for storing any
form of connected data.

We implement add, read, modify, and remove primitives for all
entities—nodes, edges, and properties. PMGD supports queries
that look up nodes or edges based on (a) a specific tag and/or
property, (b) a specific property value, or (c) a property value within
a specified range of values. Range lookups are supported for all
types of properties except blobs.

PMGD implements indexing to support all of these types of
lookups. Users can choose which properties to create indices for,
based on the expected query patterns, with an understanding that
indices occupy additional memory. PMGD also allows a query
to provide a predicate function that can examine the properties or
relationships of a node or edge and determine whether it matches the query.
Once relevant nodes have been found, PMGD supports
graph-oriented queries such as a) get neighbors of a node at n-hops,
where n >= 1; b) get all nodes within a neighborhood of up to
n-hops from a node; and c) get common neighbors of a set of nodes. Each
of these queries includes the ability to specify the direction and tag
of edges to follow.


## Library sources

The public interface headers are in the include/ folder while the
library C++ sources are in src/. The Java bindings are implemented
in the java/ folder. Some higher level functionalities like
neighbor functions are present in the util/ folder.


## Tools

We provide some simple tools like:

* **mkgraph**: to make an empty graph and provide parameters like memory
          region sizes, indexes etc. Run mkgraph -h for help.
* **loadgraph**: that can load from certain supported file formats into
            an empty graph created with mkgraph. Run loadgraph -h for help.
* **dumpgraph**: to print the contents of the graph to screen. dumpgraph -h for help.


## Tests and sample code

The test folder has unit tests for a lot of the modules and the
tests can be run using the run_all.sh script. clean_all.sh cleans
up all the graphs created by run_all.sh. We plan to move our testing
to GTEST in future release.

