# Installation Instructions

We have tested PMGD on Ubuntu systems and a version on Windows 7. But
the Windows compilation is not maintained at this point.

## Dependencies

The following dependencies must be met to correctly compile on Ubuntu 16.04:

    sudo apt-get install flex libjsoncpp-dev javacc libbison-dev openjdk-8-jdk
  
You will need to set JAVA_HOME to point to your JDK


## Compilation
After that it is a make in the main folder, with the following options.
The make file puts all the libraries in a lib/ folder.

The user must always specify whether the library is being compiled for PM
or for MSYNC as follows:

    make PMOPT=PM
    OR
    make PMOPT=MSYNC

To compile for a system that is PM-enabled (provides clwb or clflushopt):

    make PMOPT=PM PMFLUSH=clwb
    OR
    make PMOPT=PM PMFLUSH=clflushopt

The make command line always expects the PMOPT parameter. The PMFLUSH parameter is optional.
The type of cache flush instruction to use is read from /proc/cpuinfo if the PMFLUSH parameter is omitted.

The default compilation is a non-debug build with OPT set to -O3.
To compile a debug build use:

     make OPT=-g PMOPT=...



## Underlying file system

PMGD has been developed for PM-based systems which support load/store
addresseability of PM space and persistence of writes after cache
line flushes.
Our evaluation has been primarily with PM emulators using
the Persistent Memory File System
https://github.com/linux-pmfs/pmfs.
Any other PM-enabled file system that can provide the same functions
as PMFS and export a POSIX-compliant mmap call should work. We will
test this soon and update instructions here.

We also use PMGD on an ext4-based file system at the risk
of sacrificing durability. When compiled on a system that does
not support PM, the code chooses a weaker durability option where msyncs are done
on transaction commit or abort boundaries. There are Graph options that
allow a user to control when msync should be done, if at all.
One option is to perform msync for every persistent barrier by using the
AlwaysMsync option when opening a graph. It is important to understand that PMGD
has been developed with persistent memory in mind. So use on a DRAM based
ext4 system is simply to enable testing and evaluation of the system.

## How to copy a PMGD database

Since PMGD uses mmap to a large virtual memory space but maps physical pages
only when needed, the size on disk shown by some utilities can be quite
large. Similarly, copying a database can take a long time. So we use a
different method of copying a PMGD database.
Tested on Ubuntu 16.04.
Reference: https://stackoverflow.com/questions/13252682/copying-a-1tb-sparse-file

Assume a database called pmgd_db at /home/user/:

    sudo apt-get install bsdtar     # this is used for doing the sparse copy
    cd wherever_you_need_to_copy
    bsdtar cvfz pmgd_sparse.tar.gz /home/user/pmgd_db
    tar -xvSf pmgd_sparse.tar.gz

