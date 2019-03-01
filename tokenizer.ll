/* -*- Mode:c++; eval:(c-set-style "BSD"); c-basic-offset:4; indent-tabs-mode:nil; tab-width:8 -*- */

%option noyywrap
%option yylineno
%option outfile="lex.yy.cc"
 /* header-file="lex.yy.h" */

%{

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include <string>
#include "../parser.h"
#include PARSER_YY_HDR

using namespace std;

static string * strvec(const char * w, int len);

%}

%%

(\r|\n)    { /* skip nl */ }
#.*(\r|\n)+ { /* skip comments */ }
[ \t]+ { /* skip whitespace */ }

\{         { return L_CURLY; }
\}         { return R_CURLY; }
\(         { return L_PAREN; }
\)         { return R_PAREN; }

\".*\"     {
	       yylval.word = strvec(yytext+1, yyleng-2);
	       return TOK_STRING;
	   }
TABLE      { return KW_TABLE; }
BOOL       { return KW_BOOL; }
INT        { return KW_INT; }
INT64      { return KW_INT64; }
TEXT       { return KW_TEXT; }
BLOB       { return KW_BLOB; }
DOUBLE     { return KW_DOUBLE; }
INDEX      { return KW_INDEX; }
QUERY      { return KW_QUERY; }
DEFAULT    { return KW_DEFAULT; }
PROTOID    { return KW_PROTOID; }
PROTOPKG   { return KW_PROTOPKG; }
LIKEQUERY  { return KW_LIKEQUERY; }
VERSION    { return KW_VERSION; }
CUSTOM-GET { return KW_CUSTOM_GET; }
CUSTOM-UPD { return KW_CUSTOM_UPD; }
CUSTOM-DEL { return KW_CUSTOM_DEL; }

(-)?[0-9]+\.[0-9]+ {
               yylval.number_double = strtod(yytext, NULL);
               return TOK_DOUBLE;
             }

(-)?(0x)?[0-9]+ {
               yylval.number_int = strtoll(yytext, NULL, 0);
               return TOK_INTEGER;
             }

[a-zA-Z_][a-zA-Z0-9_]+ {
              yylval.word = strvec(yytext, yyleng);
              return KW_WORD;
           }

%%

void
tokenizer_init(FILE *in)
{
    yyin = in;
}

// quiet a warning about yyunput defined but not used.
void ___tokenizer__unused__crap(void)
{
  char c = 0;
  yyunput(0, &c);
}

static string *
strvec( const char * w, int len )
{
    string * ret = new string;
    ret->append(w, len);
    return ret;
}
