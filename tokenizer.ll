/* -*- Mode:c++; eval:(c-set-style "BSD"); c-basic-offset:4; indent-tabs-mode:nil; tab-width:8 -*- */

%option noyywrap
%option yylineno
%option outfile="lex.yy.cc"
 /* header-file="lex.yy.h" */

%{

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
INT        { return KW_INT; }
INT64      { return KW_INT64; }
TEXT       { return KW_TEXT; }
BLOB       { return KW_BLOB; }
DOUBLE     { return KW_DOUBLE; }
INDEX      { return KW_INDEX; }
QUERY      { return KW_QUERY; }
LIKEQUERY  { return KW_LIKEQUERY; }
CUSTOM-GET { return KW_CUSTOM_GET; }
CUSTOM-UPD { return KW_CUSTOM_UPD; }
CUSTOM-DEL { return KW_CUSTOM_DEL; }

(0x)?[0-9]+ {
               yylval.integer = strtoul(yytext, NULL, 0);
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

static string *
strvec( const char * w, int len )
{
    string * ret = new string;
    ret->append(w, len);
    return ret;
}
