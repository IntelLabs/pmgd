###
# @file   run_all.sh
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

# Exit at first sign of trouble
set -e

# Compile and run the entire test suite. Useful before checking new
# code. This will exit at the first error that happens
# *** Make sure to add new test files here by hand along with the
#     graphs they create

if test $# != 2 ; then
    echo "Usage: $0 <graphs dir> <delete existing graphs? (y/n)>"
    exit -1
fi

SCRIPT_DIR=`dirname $0`
cd $SCRIPT_DIR
SCRIPT_DIR=$PWD
cd -
echo "Script run from $SCRIPT_DIR"

GRAPH_DIR=$1
mkdir -p ${GRAPH_DIR}
# First get an absolute path for the graph dir, just in case
cd $GRAPH_DIR
GRAPH_DIR=$PWD
echo "Graphs in $GRAPH_DIR"
cd -

source ${SCRIPT_DIR}/common.sh
# This script always lives in the test folder. So make that assumption
TEST_DIR=${SCRIPT_DIR}

echo "Test dir $TEST_DIR"

DEL_GRAPH=$2

cd ..
make -s
cd -

echo "Moving to test directory $TEST_DIR and clearing old logs"
cd ${TEST_DIR}
rm -f log/*.log
mkdir -p log

# While this list could be constructed dynamically, keep it static
# to allow for choosing and making sure we don't delete wrong dirs
# by mistake.
tests=( alloctest allocaborttest avltest chunklisttest edgeindextest
        emailindextest filtertest indextest indexstringtest
        indexrangetest listtest load_gson_test load_tsv_test
        neighbortest nodeedgetest propertychunktest propertypredicatetest
        propertytest propertylisttest
        reverseindexrangetest rotest
        statsindextest statsallocatortest
        soltest stringtabletest txtest removetest
        mtalloctest stripelocktest mtavltest mtaddfindremovetest
        test720 test750 test767
        load_pmgd_tests
        BindingsTest DateTest )

graph_dirs=( fixedallocgraph varallocgraph avlgraph chunklistgraph edgeindexgraph
             fixedallocabortgraph varallocabortgraph varallocabortlargegraph
             emailindexgraph filtergraph indexgraph indexstringgraph
             indexrangegraph listgraph load_gson_graph load_tsv_graph
             neighborgraph nodeedgegraph propertychunkgraph ppgraph
             propertygraph propertylistgraph
             statsindexgraph statsallocatorgraph
             reverseindexrangegraph rograph
             solgraph stringtablegraph txgraph removegraph
             mtallocgraph mtaddfindremovegraph
             test720graph test750graph test767graph
             bindingsgraph )

if [ "${DEL_GRAPH}" = "y" ]; then
    rm_graph_dirs $GRAPH_DIR
fi

function javatest
{
    # For java, we need to be where the class is, for simplicity.
    (cd ${TEST_DIR}
    export LD_LIBRARY_PATH=../lib
    java -cp .:../lib/\* "$@" | tee >(cat >&3) | tail -1 | grep -q "Test passed"
    ) 3>&1
}

overall_status=0
echo "Launching tests"
echo "Individual test output will be stored in log/<testname>.log"
cd ${GRAPH_DIR}
for test in ${tests[@]}
do
    status=0
    echo -n "$test "

    # Assuming these arguments are ignored when the test
    # doesn't really need these. Eventually we can
    # specify the arguments in the array itself
    case "$test" in
        propertychunktest) ${TEST_DIR}/$test 1000 || status=1;;
        propertylisttest) ${TEST_DIR}/$test propertylistgraph 100000 || status=1;;
        emailindextest) ${TEST_DIR}/$test ${TEST_DIR}/email.gson || status=1;;
        removetest) ${TEST_DIR}/$test ${TEST_DIR}/allgoodcases.pmgd || status=1;;
        load_gson_test) ${TEST_DIR}/$test ${TEST_DIR}/email.gson || status=1;;
        load_tsv_test) echo "1	2
            1	3
            2	4
            3	4" | ${TEST_DIR}/$test || status=1;;
        test750)
            ${TEST_DIR}/test750 1 &&
            ${TEST_DIR}/test750 2 &&
            ${TEST_DIR}/test750 3 &&
            ${TEST_DIR}/test750 4 || status=1;;
        BindingsTest)
            # BindingsTest doesn't check whether it passed
            mkgraph bindingsgraph
            javatest $test || true;;
        DateTest)
            cmd="$test ${GRAPH_DIR}/emailindexgraph DeliveryTime 11"
            echo $cmd
            if ! javatest $cmd
            then
                echo "Test failed"
                status=1
            fi
            cmd="$test ${GRAPH_DIR}/propertygraph id7 4"
            echo $cmd
            if ! javatest $cmd
            then
                status=1
            fi
            ;;
        load_pmgd_tests) sh ${TEST_DIR}/load_pmgd_tests.sh ${GRAPH_DIR} || status=1;;
        *) ${TEST_DIR}/$test n1 n2 n3 n4 || status=1;;
    esac > ${TEST_DIR}/log/${test}.log

    if [ $status -ne 0 ]; then
        echo "FAILED"
        overall_status=1
    else
        echo "SUCCEEDED"
    fi
done

echo "Done"
exit $overall_status
