%{
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "jarvis.h"
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
"\""[^"\n]*"\""         { return cvt_string(true); }
[-a-zA-Z0-9.~!@$%_]+    { return cvt_string(false); }
[{}=,:;#]               { return yytext[0]; }
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
