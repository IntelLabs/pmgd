#!/bin/bash

prog="maketest"

usage()
{
    echo "Usage: $prog [OPTION]..."
    echo "Run a makefile test on the Jarvis Lake sources"
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
if echo $cwd | grep -vq jarvis; then
    echo "$prog: Fatal: Not inside a jarvis directory" >&2
    exit 1
fi
root=`echo $cwd | sed 's,\(.*\/jarvis\).*,\1,'`
cd $root

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
    output=`git clean -nxdf | grep -v "Would remove lib/"`
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

test $verbose -ge 1 && echo "All make tests performed successfully"
