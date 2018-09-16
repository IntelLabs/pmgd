# @file   load_pmgd_tests.sh
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

if [ $# != 1 ]; then
    echo "Need graph dir as argument"
    exit -2
fi

GRAPH_DIR=$1
SCRIPT_DIR=`dirname $0`

cd ${SCRIPT_DIR}
source ./common.sh
find_test_dir
if [ $? != 0 ]; then
    exit -2
fi
cd -

TOOLS_DIR=${TEST_DIR}/../tools
PATH=$PATH:${TOOLS_DIR}
GRAPH=load_pmgd_graph

echo "SCRIPT_DIR=${SCRIPT_DIR}, TEST_DIR=${TEST_DIR}, GRAPH_DIR=${GRAPH_DIR}"
status=0

# Eventually use dumpgraph -j <load_pmgd_graph> to generate the ideal output
# to compare with test.out
echo "Test all good cases."
mkgraph ${GRAPH_DIR}/$GRAPH
loadgraph -j ${GRAPH_DIR}/$GRAPH ${SCRIPT_DIR}/allgoodcases.pmgd
dumpgraph -j  ${GRAPH_DIR}/$GRAPH > ${SCRIPT_DIR}/tmp.pmgd
diff ${SCRIPT_DIR}/allgoodcases.pmgd ${SCRIPT_DIR}/tmp.pmgd
RET_VAL=$?
if [ $RET_VAL == 0 ]; then
    echo "Test Passed"
else
    echo "Test Failed"
    status=1
fi
rm -rf ${GRAPH_DIR}/$GRAPH  ${SCRIPT_DIR}/tmp.pmgd

## Couple more acceptable case(s)
echo "Test node/edge properties together."
mkgraph ${GRAPH_DIR}/$GRAPH
echo "21 #tag2 { id1 = 25, prop%22 = \"Hello world!\", _common = 2014-07-03T22:49:52-07:00 } 22 #tag1 { id1 = 38 } : #edge2;" | loadgraph -j -i ${GRAPH_DIR}/$GRAPH
# Create the expected output to match against
(
    cat <<EOF
21 #tag2 { id1 = 25, prop%22 = "Hello world!", _common = 2014-07-03T22:49:52-07:00 };
22 #tag1 { id1 = 38 };
21 22 : #edge2;
EOF
) > ${SCRIPT_DIR}/onegoodcase.pmgd
dumpgraph -j  ${GRAPH_DIR}/$GRAPH > ${SCRIPT_DIR}/tmp.pmgd
diff ${SCRIPT_DIR}/onegoodcase.pmgd ${SCRIPT_DIR}/tmp.pmgd
RET_VAL=$?
if [ $RET_VAL == 0 ]; then
    echo "Test Passed"
else
    echo "Test Failed"
    status=1
fi
rm -rf ${GRAPH_DIR}/$GRAPH ${SCRIPT_DIR}/tmp.pmgd ${SCRIPT_DIR}/onegoodcase.pmgd

echo "Property names starting with digit."
mkgraph ${GRAPH_DIR}/$GRAPH
echo "1 #tag2 { 12id1 = 25 };" | loadgraph -j ${GRAPH_DIR}/$GRAPH
dumpgraph -j  ${GRAPH_DIR}/$GRAPH > ${SCRIPT_DIR}/tmp.pmgd
# Create the expected output to match against
(
    cat <<EOF
1 #tag2 { 12id1 = 25 };
EOF
) > ${SCRIPT_DIR}/onegoodcase.pmgd
diff ${SCRIPT_DIR}/onegoodcase.pmgd ${SCRIPT_DIR}/tmp.pmgd
RET_VAL=$?
if [ $RET_VAL == 0 ]; then
    echo "Test Passed"
else
    echo "Test Failed"
    status=1
fi
rm -rf ${GRAPH_DIR}/$GRAPH ${SCRIPT_DIR}/tmp.pmgd ${SCRIPT_DIR}/onegoodcase.pmgd

echo "Test string 'id' values."
mkgraph ${GRAPH_DIR}/$GRAPH
echo "u21str #tag2 { id1 = 25 } u22str1! #tag1 { id1 = 38 } : #edge2 { id = 3344 };" | loadgraph -j -i ${GRAPH_DIR}/$GRAPH
dumpgraph -j  ${GRAPH_DIR}/$GRAPH > ${SCRIPT_DIR}/tmp.pmgd
# Create the expected output to match against
(
    cat <<EOF
u21str #tag2 { id1 = 25 };
u22str1! #tag1 { id1 = 38 };
u21str u22str1! : #edge2 { id = 3344 };
EOF
) > ${SCRIPT_DIR}/onegoodcase.pmgd
diff ${SCRIPT_DIR}/onegoodcase.pmgd ${SCRIPT_DIR}/tmp.pmgd
RET_VAL=$?
if [ $RET_VAL == 0 ]; then
    echo "Test Passed"
else
    echo "Test Failed"
    status=1
fi
rm -rf ${GRAPH_DIR}/$GRAPH ${SCRIPT_DIR}/tmp.pmgd ${SCRIPT_DIR}/onegoodcase.pmgd

## Echo bad cases and check for exception

# String id longer than max (16B by default. can be changed if really needed)
echo "Testing extra long string id."
mkgraph ${GRAPH_DIR}/$GRAPH
echo "1 #tagtag123456abcdefsdf #tag1 { id1 = 38 } : #edge2;" | loadgraph -j ${GRAPH_DIR}/$GRAPH 2> ${SCRIPT_DIR}/err.log
grep -q "\[Exception\]" ${SCRIPT_DIR}/err.log
RET_VAL=$?
if [ $RET_VAL == 0 ]; then
    echo "Test Passed"
else
    echo "Test Failed"
    status=1
fi
rm -rf ${GRAPH_DIR}/$GRAPH  ${SCRIPT_DIR}/err.log
exit $status
