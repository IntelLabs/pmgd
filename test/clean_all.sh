###
# @file   clean_all.sh
#
# @section LICENSE
#
# The MIT License
#
# @copyright Copyright (c) 2017 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

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
graph_dirs="$graph_dirs region1 region2"
rm_graph_dirs $GRAPH_DIR
