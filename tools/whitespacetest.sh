#!/bin/sh
#
# Must be invoked from the project's root directory.

success=0           # success until otherwise detected

for f in `find . -name \*.h -o -name \*.cc -o -name Make\* -o -name \*.bat`; do
    okay=0
    msg=''
    if echo $f | grep -Eqv 'Makefile$|Makeconf$' && grep -q '	' $f; then
        okay=1
        msg='tabs'
    fi
    if grep -q '[ 	]$' $f; then
        okay=1
        if test -z "$msg"; then
            msg='trailing whitespace'
        else
            msg="$msg, trailing whitespace"
        fi
    fi
    if tail --lines=1 $f | grep -q '^[  	]*$'; then
        okay=1
        if test -z "$msg"; then
            msg='trailing empty lines'
        else
            msg="$msg, trailing empty lines"
        fi
    fi
    if grep -q $'\r$' $f; then
        okay=1
        if test -z "$msg"; then
            msg='carriage returns'
        else
            msg="$msg, carriage returns"
        fi
    fi
    if test $okay -eq 1; then
        success=1
        echo $f: has $msg
    fi
done

exit $success
