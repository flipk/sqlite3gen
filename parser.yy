/* -*- Mode:c++; eval:(c-set-style "BSD"); c-basic-offset:4; indent-tabs-mode:nil; tab-width:8 -*- */

%{

#include <string>
#include <iostream>
#include "../tokenizer.h"
#include "../parser.h"

extern int yylex( void );
extern void yyerror( const std::string e );
extern int yylineno; // autogenerated by flex

#define YYDEBUG 1

using namespace std;

static TableDef * table_defs = NULL;

static void validate_table(TableDef *tb);

%}

%union
{
    std::string * word;
    WordList    * words;
    TableDef    * table;
    FieldDef    * field;
    TypeDefValue* type;
    FieldAttrs    attrs;
    int           integer;
    CustomGetUpdList * getupdlist;
}

%token L_CURLY R_CURLY L_PAREN R_PAREN KW_TABLE
%token KW_INT KW_INT64 KW_TEXT KW_BLOB KW_DOUBLE
%token KW_INDEX KW_QUERY KW_LIKEQUERY KW_WORD
%token KW_CUSTOM_GET KW_CUSTOM_UPD
%token TOK_INTEGER TOK_STRING

%type <word>    KW_WORD
%type <table>   TABLE
%type <field>   FIELDS FIELD
%type <type>    DATATYPE TYPELIST
%type <attrs>   ATTRIBUTES
%type <integer> TOK_INTEGER
%type <word>    TOK_STRING
%type <words>   WORDLIST
%type <getupdlist> CUSTOMGET CUSTOMUPD CUSTOMS

%start SCHEMA

%%

SCHEMA
	: TABLE
	{
		$1->next = table_defs;
		table_defs = $1;
	}
	| TABLE SCHEMA
	{
		$1->next = table_defs;
		table_defs = $1;
	}
	;

TABLE
	: KW_TABLE KW_WORD L_CURLY FIELDS CUSTOMS R_CURLY
	{
		$$ = new TableDef(*$2);
		$$->fields = $4;
                $$->customs = $5;
		delete $2;
                validate_table($$);
	}
	;

FIELDS
	: FIELD
	{
		$$ = $1;
	}
	| FIELD FIELDS
	{
		$1->next = $2;
		$$ = $1;
	}
	;

FIELD
	: KW_WORD DATATYPE ATTRIBUTES
	{
		$$ = new FieldDef(*$1, *$2);
		$$->attrs = $3;
                delete $2;
		delete $1;
	}
	;

CUSTOMS
	: /*nothing*/
	{
            $$ = NULL;
	}
	| CUSTOMGET CUSTOMS
	{
            $$ = $1;
            $$->next = $2;
	}
	| CUSTOMUPD CUSTOMS
	{
            $$ = $1;
            $$->next = $2;
	}
	;

CUSTOMGET
	: KW_CUSTOM_GET KW_WORD L_PAREN TYPELIST R_PAREN TOK_STRING
	{
            $$ = new CustomGetUpdList;
            $$->init_get(*$2, $4, *$6);
            delete $2;
            delete $6;
	}
	;

TYPELIST
	: /*nothing*/
	{
            $$ = NULL;
	}
	| DATATYPE TYPELIST
	{
            $$ = $1;
            $$->next = $2;
	}
	;

CUSTOMUPD
	: KW_CUSTOM_UPD KW_WORD L_PAREN WORDLIST R_PAREN
	{
            $$ = new CustomGetUpdList;
            $$->init_upd(*$2, $4);
            delete $2;
	}
	;

WORDLIST
	: /*nothing*/
	{
            $$ = NULL;
	}
	| KW_WORD WORDLIST
	{
            $$ = new WordList;
            $$->init(*$1);
            $$->next = $2;
            delete $1;
	}
	;

DATATYPE
	: KW_INT
        {
            $$ = new TypeDefValue;
            $$->init(TYPE_INT);
        }
	| KW_INT64
        {
            $$ = new TypeDefValue;
            $$->init(TYPE_INT64);
        }
	| KW_TEXT
        {
            $$ = new TypeDefValue;
            $$->init(TYPE_TEXT);
        }
	| KW_DOUBLE
        {
            $$ = new TypeDefValue;
            $$->init(TYPE_DOUBLE);
        }
	| KW_BLOB
        {
            $$ = new TypeDefValue;
            $$->init(TYPE_BLOB);
        }
	;

ATTRIBUTES
	: /*nothing*/
	{
		$$.init();
	}
	| ATTRIBUTES KW_INDEX
	{
		$$ = $1;
		$$.index = true;
	}
	| ATTRIBUTES KW_QUERY
	{
		$$ = $1;
		$$.query = true;
	}
	| ATTRIBUTES KW_LIKEQUERY
	{
		$$ = $1;
		$$.likequery = true;
	}
	;

%%

void
yyerror( const string e )
{
    fprintf(stderr, "error: %d: %s\n", yylineno, e.c_str());
    exit( 1 );
}

TableDef *
parse_file(const std::string &fname)
{
    FILE * f = fopen(fname.c_str(), "r");
    if (f == NULL)
    {
        fprintf(stderr, "cannot open file '%s'\n", fname.c_str());
        return NULL;
    }
    table_defs = NULL;
    tokenizer_init(f);
    yyparse();
    fclose(f);
    return table_defs;
}

void
print_tokenized_file(const std::string &fname)
{
    FILE * f = fopen(fname.c_str(), "r");
    if (f == NULL)
    {
        fprintf(stderr, "cannot open file '%s'\n", fname.c_str());
        return;
    }
    tokenizer_init(f);
    while (1)
    {
        int c = yylex();

        cout << "got " << c
	     << " ("
	     << (c <= YYMAXUTOK ? yytname[yytranslate[c]] : "")
             << ")";
        if (c == KW_WORD)
            cout << " (" << *yylval.word << ")";
        cout << endl;
        if (c <= 0)
            break;
    }
    fclose(f);
}

const char *
get_type(TypeDef type)
{
    switch (type)
    {
    case TYPE_INT:    return "int";
    case TYPE_INT64:  return "int64";
    case TYPE_TEXT:   return "text";
    case TYPE_BLOB:   return "blob";
    case TYPE_DOUBLE: return "double";
    }
    return "UNKNOWN";
}
void
print_field(FieldDef *fd)
{
    printf("  field %s i %d q %d lq %d type %s\n",
           fd->name.c_str(),
           fd->attrs.index, fd->attrs.query, fd->attrs.likequery,
           get_type(fd->type.type));
}

void
print_table(TableDef *td)
{
    printf("TABLE %s\n", td->name.c_str());
    FieldDef * fd = td->fields;
    while (fd)
    {
        print_field(fd);
        fd = fd->next;
    }
    CustomGetUpdList * cust = NULL;
    for (cust = td->customs; cust; cust = cust->next)
    {
        if (cust->type == CustomGetUpdList::GET)
        {
            printf("custom GET : get_by_%s (", cust->name.c_str());
            TypeDefValue * type;
            for (type = cust->typelist; type; type = type->next)
            {
                printf("%s", get_type(type->type));
                if (type->next)
                    printf(", ");
            }
            printf(") (\"%s\")\n", cust->query.c_str());
        }
        else
        {
            printf("custom UPD : update_%s (", cust->name.c_str());
            WordList * word;
            for (word = cust->wordlist; word; word = word->next)
            {
                printf("%s", word->word.c_str());
                if (word->next)
                    printf(", ");
            }
            printf(")\n");
        }
    }
}

void
print_tables(TableDef *tds)
{
    while (tds)
    {
        print_table(tds);
        tds = tds->next;
    }
}

static bool
is_column(TableDef *tb, const std::string &name)
{
    FieldDef * f;
    for (f = tb->fields; f; f = f->next)
        if (name == f->name)
            return true;
    return false;
}

static void
validate_table(TableDef *tb)
{
    CustomGetUpdList * cust;

    // validate the 'words' in any CUSTOM-UPD are
    // actually column names from the table.
    for (cust = tb->customs; cust; cust = cust->next)
    {
        if (cust->type == CustomGetUpdList::UPD)
        {
            WordList * w;
            for (w = cust->wordlist; w; w = w->next)
            {
                if (!is_column(tb, w->word))
                {
                    fprintf(stderr, "ERROR: column '%s' in CUSTOM-UPD "
                            "is not a known column in table '%s'\n",
                            w->word.c_str(), tb->name.c_str());
                    exit(1);
                }
            }
        }
    }
}
