# @file   common.sh
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
# Common functions used by other scripts like run_all.sh, clean_all.sh....
# *** Please do not execute this file directly. Assumes state passed around
# Use source common.sh where functions required

find_test_dir()
{
    CURR_DIR=`basename $PWD`
    # Search only in the restricted set
    PMGD_DIRS=( "test" "src" "tools" "util" "include" "lib" )
    echo ${PMGD_DIRS[@]} | grep ${CURR_DIR}
    if [ $? -eq 0 ]; then
        TEST_DIR=../test
    else
        # Pick the first pmgd git folder
        TEST_DIR=`find ~ -type d -name ".git" | grep -w pmgd | xargs -n 1 dirname 2>/dev/null | head`/test
        if [ "$TEST_DIR" = "/test" ]; then
            echo "No pmgd folder found"
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
