# @file   whitespacetest.sh
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
