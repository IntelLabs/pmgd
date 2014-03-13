#!/bin/sh
#
# Check that any given include file includes all the necessary other
# include files needed for its definitions and declarations.
#
# Must be invoked from the project's root directory.

success=0           # success until otherwise detected

pushd src > /dev/null
for f in ../include/*.h ../src/*.h; do
    echo "#include \"$f\"" > x.cc
    if ! make x.o > Out 2>&1; then
        success=1
        echo "$f: Problematic include file (see details below)"
        echo
        cat Out
    fi
    rm -f x.cc x.d x.o Out
done

exit $success
