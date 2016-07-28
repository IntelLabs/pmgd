#!/bin/bash

SCRIPT_DIR=`dirname $0`
GRAPH_DIR=$SCRIPT_DIR

source ${SCRIPT_DIR}/common.sh
find_test_dir
if [ $? != 0 ]; then
    exit -2
fi

TOOLS_DIR=${TEST_DIR}/../tools
PATH=$PATH:${TOOLS_DIR}
GRAPH=load_jarvis_graph

echo "SCRIPT_DIR=${SCRIPT_DIR}, TEST_DIR=${TEST_DIR}, GRAPH_DIR=${GRAPH_DIR}"
status=0

# Eventually use dumpgraph -j <load_jarvis_graph> to generate the ideal output
# to compare with test.out
echo "Test all good cases."
mkgraph ${GRAPH_DIR}/$GRAPH
loadgraph -j ${GRAPH_DIR}/$GRAPH ${SCRIPT_DIR}/allgoodcases.jarvis
dumpgraph -j  ${GRAPH_DIR}/$GRAPH > ${SCRIPT_DIR}/tmp.jarvis
diff ${SCRIPT_DIR}/allgoodcases.jarvis ${SCRIPT_DIR}/tmp.jarvis
RET_VAL=$?
if [ $RET_VAL == 0 ]; then
    echo "Test Passed"
else
    echo "Test Failed"
    status=1
fi
rm -rf ${GRAPH_DIR}/$GRAPH  ${SCRIPT_DIR}/tmp.jarvis

## Couple more acceptable case(s)
echo "Test node/edge properties together."
mkgraph ${GRAPH_DIR}/$GRAPH
echo "21 #tag2 { id1 = 25, prop%22 = \"Hello world!\", _common = 2014-07-03T22:49:52-07:00 } 22 #tag1 { id1 = 38 } : #edge2;" | loadgraph -j ${GRAPH_DIR}/$GRAPH
# Create the expected output to match against
(
    cat <<EOF
21 #tag2 { id1 = 25, prop%22 = "Hello world!", _common = 2014-07-03T22:49:52-07:00 };
22 #tag1 { id1 = 38 };
21 22 : #edge2;
EOF
) > ${SCRIPT_DIR}/onegoodcase.jarvis
dumpgraph -j  ${GRAPH_DIR}/$GRAPH > ${SCRIPT_DIR}/tmp.jarvis
diff ${SCRIPT_DIR}/onegoodcase.jarvis ${SCRIPT_DIR}/tmp.jarvis
RET_VAL=$?
if [ $RET_VAL == 0 ]; then
    echo "Test Passed"
else
    echo "Test Failed"
    status=1
fi
rm -rf ${GRAPH_DIR}/$GRAPH ${SCRIPT_DIR}/tmp.jarvis ${SCRIPT_DIR}/onegoodcase.jarvis

echo "Property names starting with digit."
mkgraph ${GRAPH_DIR}/$GRAPH
echo "1 #tag2 { 12id1 = 25 };" | loadgraph -j ${GRAPH_DIR}/$GRAPH
dumpgraph -j  ${GRAPH_DIR}/$GRAPH > ${SCRIPT_DIR}/tmp.jarvis
# Create the expected output to match against
(
    cat <<EOF
1 #tag2 { 12id1 = 25 };
EOF
) > ${SCRIPT_DIR}/onegoodcase.jarvis
diff ${SCRIPT_DIR}/onegoodcase.jarvis ${SCRIPT_DIR}/tmp.jarvis
RET_VAL=$?
if [ $RET_VAL == 0 ]; then
    echo "Test Passed"
else
    echo "Test Failed"
    status=1
fi
rm -rf ${GRAPH_DIR}/$GRAPH ${SCRIPT_DIR}/tmp.jarvis ${SCRIPT_DIR}/onegoodcase.jarvis

echo "Test string 'id' values."
mkgraph ${GRAPH_DIR}/$GRAPH
echo "u21str #tag2 { id1 = 25 } u22str1! #tag1 { id1 = 38 } : #edge2 { id = 3344 };" | loadgraph -j ${GRAPH_DIR}/$GRAPH
dumpgraph -j  ${GRAPH_DIR}/$GRAPH > ${SCRIPT_DIR}/tmp.jarvis
# Create the expected output to match against
(
    cat <<EOF
u21str #tag2 { id1 = 25 };
u22str1! #tag1 { id1 = 38 };
u21str u22str1! : #edge2 { id = 3344 };
EOF
) > ${SCRIPT_DIR}/onegoodcase.jarvis
diff ${SCRIPT_DIR}/onegoodcase.jarvis ${SCRIPT_DIR}/tmp.jarvis
RET_VAL=$?
if [ $RET_VAL == 0 ]; then
    echo "Test Passed"
else
    echo "Test Failed"
    status=1
fi
rm -rf ${GRAPH_DIR}/$GRAPH ${SCRIPT_DIR}/tmp.jarvis ${SCRIPT_DIR}/onegoodcase.jarvis

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
