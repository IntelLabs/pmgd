#!/bin/bash
# Common functions used by other scripts like run_all.sh, clean_all.sh....
# *** Please do not execute this file directly. Assumes state passed around
# Use source common.sh where functions required

find_test_dir()
{
    CURR_DIR=`basename $PWD`
    # Search only in the restricted set
    JARVIS_DIRS=( "test" "src" "tools" "util" "include" "lib" )
    echo ${JARVIS_DIRS[@]} | grep ${CURR_DIR}
    if [ $? -eq 0 ]; then
        TEST_DIR=../test
    else
        # Pick the first jarvis git folder
        TEST_DIR=`find ~ -type d -name ".git" | grep -w jarvis | xargs -n 1 dirname 2>/dev/null | head`/test
        if [ "$TEST_DIR" = "/test" ]; then
            echo "No jarvis folder found"
            TEST_DIR=$PWD
            return 2
        fi
    fi
}

rm_graph_dirs()
{
    if test $# != 1; then
        echo "This function needs the graph directory"
        return 1
    fi
    GRAPH_DIR=$1
    cd $GRAPH_DIR
    for dir in ${graph_dirs[@]}
    do
        echo "Removing graph: $dir"
        rm -rf $dir
    done
    cd -
}
