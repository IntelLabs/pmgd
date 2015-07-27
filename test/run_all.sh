#!/bin/bash
# Compile and run the entire test suite. Useful before checking new
# code. This will exit at the first error that happens 
# *** Make sure to add new test files here by hand along with the
#     graphs they create

if test $# != 2 ; then
    echo "Usage: $0 <graphs dir> <delete existing graphs? (y/n)>"
    exit -1
fi

SCRIPT_DIR=`dirname $0`
GRAPH_DIR=$1
mkdir -p ${GRAPH_DIR}
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

DEL_GRAPH=$2

cd ..
make -s
cd -

echo "Moving to test directory $TEST_DIR and clearing old logs"
cd ${TEST_DIR}
rm -f log/*.log
mkdir -p log

# Exit at first sign of trouble
set -e

# While this list could be constructed dynamically, keep it static
# to allow for choosing and making sure we don't delete wrong dirs
# by mistake.
tests=( alloctest avltest chunklisttest edgeindextest
        emailindextest filtertest indextest indexstringtest
        listtest load_gson_test load_tsv_test
        neighbortest nodeedgetest propertychunktest propertypredicatetest
        propertytest propertylisttest
        reverseindexrangetest rotest
        soltest stringtabletest txtest
        test720 test750 test767 
        DateTest )

graph_dirs=( alloctestdummy avlgraph chunklistgraph edgeindexgraph
             emailindexgraph filtergraph indexgraph indexstringgraph
             listgraph load_gson_graph load_tsv_graph
             neighborgraph nodeedgegraph propertychunkgraph ppgraph
             propertygraph propertylistgraph
             reverseindexrangegraph rograph
             solgraph stringtablegraph txgraph
             test720graph test750graph test767graph )

if [ "${DEL_GRAPH}" = "y" ]; then
    rm_graph_dirs $GRAPH_DIR
fi

set +e

echo "Launching tests"
echo "Individual test output will be stored in log/<testname>.log"
cd ${GRAPH_DIR}
for test in ${tests[@]}
do
    echo -n "$test "

    # Assuming these arguments are ignored when the test
    # doesn't really need these. Eventually we can
    # specify the arguments in the array itself
    case "$test" in
        propertychunktest) ${TEST_DIR}/$test 1000;;
        propertylisttest) ${TEST_DIR}/$test propertylistgraph 100000;;
        load_gson_test) ${TEST_DIR}/$test email.gson;;
        load_tsv_test) echo "1	2
            1	3
            2	4
            3	4" | ${TEST_DIR}/$test;;
        test750)
            ${TEST_DIR}/test750 1 &&
            ${TEST_DIR}/test750 2 &&
            ${TEST_DIR}/test750 3 &&
            ${TEST_DIR}/test750 4;;
        DateTest)
            # For java, we need to be where the class is, for simplicity.
            cd ${TEST_DIR}
            RESULT1=`LD_LIBRARY_PATH=../lib java -cp .:../lib/\* $test ${GRAPH_DIR}/emailindexgraph "DeliveryTime" 13 | tail -n 1`
            RESULT2=`LD_LIBRARY_PATH=../lib java -cp .:../lib/\* $test ${GRAPH_DIR}/propertygraph "id7" 4 | tail -n 1`
            if [ "$RESULT1" = "Test passed" ] && [ "$RESULT2" = "Test passed" ]; then
                echo "PASSED"
            else
                # Command to create an error return since Java doesn't
                # allow main to return a value
                rm emailindexgraph 2> /dev/zero
            fi
            cd -
            ;;
        *) ${TEST_DIR}/$test n1 n2 n3 n4 ;;
    esac > ${TEST_DIR}/log/${test}.log

    if [ $? -ne 0 ]; then
        echo "FAILED"
    else
        echo "SUCCEEDED"
    fi
done

echo "Done"
