/**
 * @file   maketest.sh
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#!/bin/bash

prog="maketest"

usage()
{
    echo "Usage: $prog [OPTION]..."
    echo "Run a makefile test on the PMGD sources"
    echo ""
    echo "  -v=LEVEL  set verbose level (0, 1, 2)"
    echo "  -h        print this message"
}

verbose=0
for i in "$@"; do
    case $i in
        -v=[0-9]) verbose="${i/-v=/}"; shift;;
        -h)       usage; exit 0;;
        -*)       echo "$prog: invalid option -- '$i'" >&2; usage >&2; exit 1;;
    esac
done

cwd=`pwd`
if echo $cwd | grep -vq pmgd; then
    echo "$prog: Fatal: Not inside a pmgd directory" >&2
    exit 1
fi
root=`echo $cwd | sed 's,\(.*\/pmgd\).*,\1,'`
cd $root

cleanuplist=""
trap cleanup 0

cleanup()
{
    rm -rf $cleanuplist
}

onerror()
{
    echo "$prog: Failure: $reason" >&2
    exit 1
}

# The second "make all" invocation shouldn't have to do anything.
test_make_all_twice()
{
    test $verbose -ge 2 && echo "Make all twice test"
    reason="Make all twice test: Something unexpected had to be done"
    make -s clean > /dev/null
    make -s > /dev/null
    output=`make -s`
    if test ! -z "$output"; then
        return 1
    fi
}

# The Makefile should know how to build any singular .o
test_singular_target_from_root()
{
    test $verbose -ge 2 && echo "Make every single .o from an empty repository"
    base_reason="Singular target test from root"
    for file in */*.{cc,l,y}; do
        target=`echo $file | sed 's,\...*,.o,'`
        make -s clean > /dev/null
        reason="$base_reason: $target: Failed to build"
        make -s $target > /dev/null
        if test $? -ne 0; then
            return 1
        fi
    done
}

# After clean, there shouldn't be anything left
test_make_clean()
{
    test $verbose -ge 2 && echo "Make clean test"
    reason="Make clean test: Clean failed to delete some files"
    make -s clean > /dev/null
    make -s > /dev/null
    make -s clean > /dev/null
    output=`git clean -nxdf`
    if test ! -z "$output"; then
        return 1
    fi
}

# After touching a G++ source file, make should redo at least one target
test_touch_gxx()
{
    test $verbose -ge 2 && echo "Touch a .cc file and make test"
    reason="Touch a C++ file then make all test: Nothing was done"
    make -s clean > /dev/null
    make -s > /dev/null
    touch src/graph.cc
    output=`make -s`
    if test -z "$output"; then
        return 1
    fi
}

# After touching a lex source file, make should redo at least one target
test_touch_lex()
{
    test $verbose -ge 2 && echo "Touch a .l file and make test"
    reason="Touch a lex file then make all test: Nothing was done"
    make -s clean > /dev/null
    make -s > /dev/null
    touch util/scanner.l
    output=`make -s`
    if test -z "$output"; then
        return 1
    fi
}

# After touching a yacc source file, make should redo at least one target
test_touch_yacc()
{
    test $verbose -ge 2 && echo "Touch a .y file and make test"
    reason="Touch a yacc file then make all test: Nothing was done"
    make -s clean > /dev/null
    make -s > /dev/null
    touch util/loader.y
    output=`make -s`
    if test -z "$output"; then
        return 1
    fi
}

# After touching an intermediate file, make should redo at least one target
test_touch_intermediate()
{
    test $verbose -ge 2 && echo "Touch an intermediate file and make test"
    reason="Touch an intermediate file then make all test: Nothing was done"
    make -s clean > /dev/null
    make -s > /dev/null
    touch util/scanner.cc
    output=`make -s`
    if test -z "$output"; then
        return 1
    fi
}

# Trying building mkgraph
test_make_mkgraph()
{
    test $verbose -ge 2 && echo "Make mkgraph test"
    reason="Build mkgraph test: Nothing was done or build error"
    make -s clean > /dev/null
    output=`make -s tools/mkgraph`
    if test $? -ne 0 -o -z "$output"; then
        return 1
    fi
}

# Trying to build to a remote location
test_obuild()
{
    test $verbose -ge 2 && echo "Remote build test"
    reason="Remote build test: Build error"
    make -s clean > /dev/null
    cleanuplist=`mktemp -d /tmp/maketestXXXX`
    output=`make -s O=$cleanuplist`
    status=$?
    cleanup
    if test $status -ne 0 -o -z "$output"; then
        return 1
    fi
}

# Trying to clean after a remote location
test_obuild_clean()
{
    test $verbose -ge 2 && echo "Remote clean test"
    reason="Remote clean test: Clean failed or not thorough"
    make -s clean > /dev/null
    cleanuplist=`mktemp -d /tmp/maketestXXXX`
    make -s O=$cleanuplist > /dev/null
    make -s O=$cleanuplist clean > /dev/null
    status=$?
    exist=0
    `test -e $cleanuplist` && exist=1
    cleanup
    if test $status -ne 0 -o $exist -eq 1; then
        return 1
    fi
}

# Touching an intermediate file should cause make to rebuild a few pieces
test_obuild_touch()
{
    test $verbose -ge 2 && echo "Touch intermediate file and remote make test"
    reason="Remote touch test: Build failed or did nothing"
    make -s clean > /dev/null
    cleanuplist=`mktemp -d /tmp/maketestXXXX`
    make -s O=$cleanuplist > /dev/null
    touch $cleanuplist/util/loader.cc
    output=`make -s O=$cleanuplist`
    status=$?
    cleanup
    if test $status -ne 0 -o -z "$output"; then
        return 1
    fi
}

# Trying to build propertytest at a remote location
test_obuild_make_propertytest()
{
    test $verbose -ge 2 && echo "Make remote propertytest test"
    reason="Remote build propertytest test: Build failed or didn't build target"
    make -s clean > /dev/null
    cleanuplist=`mktemp -d /tmp/maketestXXXX`
    make -s O=$cleanuplist test/propertytest > /dev/null
    status=$?
    binary=0
    test -x $cleanuplist/test/propertytest && binary=1
    cleanup
    if test $status -ne 0 -o $binary -eq 0; then
        return 1
    fi
}

# Trying building src
test_make_src()
{
    test $verbose -ge 2 && echo "Make src test"
    reason="Build src test: Nothing was done or build error"
    make -s clean > /dev/null
    output=`make -s src`
    if test $? -ne 0 -o -z "$output"; then
        return 1
    fi
}

# Trying building util
test_make_util()
{
    test $verbose -ge 2 && echo "Make util test"
    reason="Build util test: Nothing was done or build error"
    make -s clean > /dev/null
    output=`make -s util`
    if test $? -ne 0 -o -z "$output"; then
        return 1
    fi
}

# Trying building tools
test_make_tools()
{
    test $verbose -ge 2 && echo "Make tools test"
    reason="Build tools test: Nothing was done or build error"
    make -s clean > /dev/null
    output=`make -s tools`
    if test $? -ne 0 -o -z "$output"; then
        return 1
    fi
}

# Trying building test
test_make_test()
{
    test $verbose -ge 2 && echo "Make test test"
    reason="Build test test: Nothing was done or build error"
    make -s clean > /dev/null
    output=`make -s test`
    if test $? -ne 0 -o -z "$output"; then
        return 1
    fi
}

# Trying building default from src
test_make_default_from_src()
{
    test $verbose -ge 2 && echo "Make default from src test"
    reason="Build all from src test: Nothing was done or build error"
    make -s clean > /dev/null
    output=`make --no-print-directory -s -C src`
    if test $? -ne 0 -o -z "$output"; then
        return 1
    fi
}

# Trying building util from test
test_make_util_from_test()
{
    test $verbose -ge 2 && echo "Make util from test test"
    reason="Build util from test test: Nothing was done or build error"
    make -s clean > /dev/null
    output=`make --no-print-directory -s -C test util`
    if test $? -ne 0 -o -z "$output"; then
        return 1
    fi
}

# Trying building dumpgraph from tools
test_make_dumpgraph_from_tools()
{
    test $verbose -ge 2 && echo "Make dumpgraph from tools test"
    reason="Build dumpgraph from tools test: Nothing was done or build error"
    make -s clean > /dev/null
    output=`make --no-print-directory -s -C tools dumpgraph`
    if test $? -ne 0 -o -z "$output"; then
        return 1
    fi
}

test_make_all_twice
if test $? -ne 0; then
    onerror
fi

test_singular_target_from_root
if test $? -ne 0; then
    onerror
fi

test_make_clean
if test $? -ne 0; then
    onerror
fi

test_touch_gxx
if test $? -ne 0; then
    onerror
fi

test_touch_lex
if test $? -ne 0; then
    onerror
fi

test_touch_yacc
if test $? -ne 0; then
    onerror
fi

test_touch_intermediate
if test $? -ne 0; then
    onerror
fi

test_make_mkgraph
if test $? -ne 0; then
    onerror
fi

test_obuild
if test $? -ne 0; then
    onerror
fi

test_obuild_clean
if test $? -ne 0; then
    onerror
fi

test_obuild_touch
if test $? -ne 0; then
    onerror
fi

test_obuild_make_propertytest
if test $? -ne 0; then
    onerror
fi

test_make_src
if test $? -ne 0; then
    onerror
fi

test_make_util
if test $? -ne 0; then
    onerror
fi

test_make_tools
if test $? -ne 0; then
    onerror
fi

test_make_test
if test $? -ne 0; then
    onerror
fi

test_make_default_from_src
if test $? -ne 0; then
    onerror
fi

test_make_util_from_test
if test $? -ne 0; then
    onerror
fi

test_make_dumpgraph_from_tools
if test $? -ne 0; then
    onerror
fi

test $verbose -ge 1 && echo "All make tests performed successfully"
