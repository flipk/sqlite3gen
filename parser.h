/* -*- Mode:c++; eval:(c-set-style "BSD"); c-basic-offset:4; indent-tabs-mode:nil; tab-width:8 -*- */

#ifndef __PARSER_H__
#define __PARSER_H__ 1

#include <inttypes.h>
#include <vector>

enum TypeDef
{
    TYPE_INT    = 1,
    TYPE_INT64  = 2,
    TYPE_TEXT   = 3,
    TYPE_BLOB   = 4,
    TYPE_DOUBLE = 5,
    TYPE_BOOL   = 6,
    TYPE_ENUM   = 7,
    TYPE_SUBTABLE = 8
};

struct TypeDefValue
{
    struct TypeDefValue * next;
    TypeDef  type;
    std::string enum_name;
    void init(TypeDef _type) {
        type = _type;
    }
    TypeDefValue(void) {
        next = NULL;
    }
    ~TypeDefValue(void) {
        if (next)
            delete next;
    }
};

struct WordList {
    struct WordList * next;
    std::string word;
    void init(const std::string &_word) {
        word = _word;
    }
    WordList(void) {
        next = NULL;
    }
    ~WordList(void) {
        if (next)
            delete next;
    }
};

struct CustomGetUpdList {
    struct CustomGetUpdList * next;
    enum { GET, UPD, UPDBY, DEL } type;
    std::string name;
    TypeDefValue *typelist; // for GET/DEL/UPDBY
    std::string query; // for GET/DEL/UPDBY
    WordList *wordlist; // for UPD/UPDBY
    void init_get(const std::string &_name, TypeDefValue *_typelist,
                  const std::string &_query) {
        type = GET;
        name = _name;
        typelist = _typelist;
        query = _query;
    }
    void init_del(const std::string &_name, TypeDefValue *_typelist,
                  const std::string &_query) {
        type = DEL;
        name = _name;
        typelist = _typelist;
        query = _query;
    }
    void init_upd(const std::string &_name, WordList *_wordlist) {
        type = UPD;
        name = _name;
        wordlist = _wordlist;
    }
    void init_updby(const std::string &_name, WordList *_wordlist,
                    TypeDefValue *_typelist, const std::string &_query) {
        type = UPDBY;
        name = _name;
        wordlist = _wordlist;
        typelist = _typelist;
        query = _query;
    }
    CustomGetUpdList(void) {
        next = NULL;
        typelist = NULL;
        wordlist = NULL;
    }
    ~CustomGetUpdList(void) {
        if (next)
            delete next;
        if (typelist)
            delete typelist;
        if (wordlist)
            delete wordlist;
    }
};

struct FieldAttrs
{
    bool index;
    bool query;
    bool likequery;
    bool auto_increment;
    std::string init_string;
    int64_t init_int;
    double init_double;
    int protoid;
    bool foreign;
    std::string foreign_table;
    std::string foreign_field;
    bool subtable;
    std::string constraints;
    struct TableDef *subtable_table;
    struct FieldDef *subtable_field;
    void init(void) {
        index = false;
        query = false;
        likequery = false;
        auto_increment = false;
        init_string.clear();
        init_int = 0;
        init_double = 0.0;
        protoid = -1;
        foreign = false;
        subtable = false;
        subtable_table = NULL;
        subtable_field = NULL;
        constraints.clear();
    }
    FieldAttrs(void) {
        init();
    }
    ~FieldAttrs(void) {
    }
};

struct FieldDef
{
    struct FieldDef * next;
    std::string name;
    TypeDefValue type;
    FieldAttrs attrs;
    FieldDef(const std::string &_name, TypeDefValue _type)
        : name(_name), type(_type)
    {
        next = NULL;
        attrs.init();
    }
    ~FieldDef(void)
    {
        if (next)
            delete next;
    }
};

struct TableDef
{
    struct TableDef * next;
    std::string name;
    int version;
    bool is_subtable;
    FieldDef * fields;
    std::string constraints;
    CustomGetUpdList * customs;
    TableDef(const std::string &_name)
        : name(_name)
    {
        next = NULL;
        fields = NULL;
        customs = NULL;
        is_subtable = false;
    }
    ~TableDef(void)
    {
        if (next)
            delete next;
        if (fields)
            delete fields;
        if (customs)
            delete customs;
    }
};

struct CustomSelect
{
    struct CustomSelect * next;
    std::string name;
    WordList * field_names;
    std::vector<TableDef*> field_table_ptrs;
    // note, this is gross, but i don't have a better
    // way to do it right now: a null field ptr means
    // "rowid" since i don't actually track rowid as
    // a field.
    std::vector<FieldDef*> field_ptrs;
    WordList * table_names;
    std::vector<TableDef*> table_ptrs;
    TypeDefValue * types;

    // there are two CUSTOM-SELECT syntaxes, one with
    // only a where-clause, and one with the full statement
    // (after field list). only one of the two following
    // fields of this struct will be populated. in the event
    // of the full_statement case, table_names is not populated.
    std::string where_clause;
    std::string full_statement;

    CustomSelect(void) {
        next = NULL;
    }
    ~CustomSelect(void) {
        if (next)
            delete next;
    }
};

struct SchemaDef
{
    std::string fname;
    std::string package;
    std::string headertop;
    std::string headerbottom;
    std::string sourcetop;
    std::string sourcebottom;
    std::string prototop;
    std::string protobottom;
    struct TableDef * tables;
    struct TableDef ** tables_next;
    struct CustomSelect * custom_selects;
    struct CustomSelect ** custom_selects_next;
    SchemaDef(void) {
        tables = NULL;
        tables_next = &tables;
        custom_selects = NULL;
        custom_selects_next = &custom_selects;
    }
    ~SchemaDef(void) {
        if (tables)
            delete tables;
        if (custom_selects)
            delete custom_selects;
    }
    void add_table(struct TableDef *tb) {
        *tables_next = tb;
        tables_next = &tb->next;
    }
    void add_custom_select(struct CustomSelect *csel) {
        *custom_selects_next = csel;
        custom_selects_next = &csel->next;
    }
};

SchemaDef * parse_file(const std::string &fname);
void print_tokenized_file(const std::string &fname);
void print_schema(SchemaDef *schema);

#endif /* __PARSER_H__ */
