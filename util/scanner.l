/**
 * @file   scanner.l
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

%{
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "pmgd.h"
#include "loader.h"
#include "loader.hh"

#undef yywrap
#define yywrap() 1

static int cvt_int();
static int cvt_string(bool quoted);
%}

%option noyywrap
%option nounput

HEX_NUMBER 0x[0-9a-fA-F]+
OCT_NUMBER 0[0-7]*
DEC_NUMBER [1-9][0-9]*
NUMBER -?({HEX_NUMBER}|{OCT_NUMBER}|{DEC_NUMBER})

%%

[ \t\n]   { ; }

true                    { return TRUE; }
false                   { return FALSE; }

{NUMBER}                { return cvt_int(); }
\"([^"\n\\]|\\["\\])*\" { return cvt_string(true); }
[{}=,:;#]               { return yytext[0]; }
[-a-zA-Z0-9.~!@$%_?:+]+ { return cvt_string(false); }
.                       { return ERROR; }

%%

static int cvt_int()
{
    char *e;
    long long v = strtoll(yytext, &e, 0);
    assert(*e == '\0');
    yylval.i = v;
    return INTEGER;
}

static int cvt_string(bool quoted)
{
    const char *p = yytext;
    int len = strlen(p);

    if (quoted)
        p += 1, len -= 2;

    yylval.s = new std::string(p, len);
    return quoted ? QUOTED_STRING : STRING;
}
