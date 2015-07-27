#!/bin/bash
# Clean executables and intermediate directories

if test $# != 1 ; then
    echo "Usage: $0 <graph dir>"
    exit -1
fi

GRAPH_DIR=$1
SCRIPT_DIR=`dirname $0`

# First get an absolute path for the graph dir, just in case
cd $GRAPH_DIR
GRAPH_DIR=$PWD
echo "Graphs in $GRAPH_DIR"
cd -

source ${SCRIPT_DIR}/common.sh
find_test_dir
if [ $? != 0 ]; then
    exit -2
fi

echo "Moving to test directory and clearing everything"
cd ${TEST_DIR}
# Logs
rm log/*.log
rmdir log

# Executables. This should remove the loadgraph and dumpgraph execs
cd ..
make clean

cd ${GRAPH_DIR}

# Intermediate graphs
graph_dirs=`find -iname "*graph" -type d`  
graph_dirs="$graph_dirs alloctestdummy region1"
rm_graph_dirs $GRAPH_DIR
