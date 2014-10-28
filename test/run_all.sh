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
source ${SCRIPT_DIR}/common.sh
find_test_dir
if [ $? != 0 ]; then
    exit -2
fi

GRAPH_DIR=$1
DEL_GRAPH=$2

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
        soltest stringtabletest txtest )

graph_dirs=( alloctestdummy avlgraph chunklistgraph edgeindexgraph
             emailindexgraph filtergraph indexgraph indexstringgraph
             listgraph load_gson_graph load_tsv_graph
             neighborgraph nodeedgegraph propertychunkgraph ppgraph
             propertygraph propertylistgraph
             reverseindexrangegraph rograph
             solgraph stringtablegraph txgraph )

#make -s clean
make -s

if [ "${DEL_GRAPH}" = "y" ]; then
    rm_graph_dirs $GRAPH_DIR
fi

set +e

echo "Launching tests"
echo "Individual test output will be stored in log/<testname>.log"
for test in ${tests[@]}
do
    echo -n "$test "

    # Assuming these arguments are ignored when the test
    # doesn't really need these. Eventually we can
    # specify the arguments in the array itself
    case "$test" in
        propertychunktest) ./$test 1000;;
        propertylisttest) ./$test propertylistgraph 100000;;
        load_gson_test) ./$test email.gson;;
        load_tsv_test) echo "1	2
            1	3
            2	4
            3	4" | ./$test;;
        *) ./$test n1 n2 n3 n4 ;;
    esac > ./log/${test}.log

    if [ $? -ne 0 ]; then
        echo "FAILED"
    else
        echo "SUCCEEDED"
    fi
done

echo "Done"
