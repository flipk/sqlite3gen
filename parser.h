/* -*- Mode:c++; eval:(c-set-style "BSD"); c-basic-offset:4; indent-tabs-mode:nil; tab-width:8 -*- */

#ifndef __PARSER_H__
#define __PARSER_H__ 1

enum TypeDef
{
    TYPE_INT    = 1,
    TYPE_INT64  = 2,
    TYPE_TEXT   = 3,
    TYPE_BLOB   = 4,
    TYPE_DOUBLE = 5
};

struct TypeDefValue
{
    struct TypeDefValue * next;
    TypeDef  type;
// TODO initial value
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
    enum { GET, UPD } type;
    std::string name;
    TypeDefValue *typelist; // for GET
    std::string query; // for GET only
    WordList *wordlist; // for UPD
    void init_get(const std::string &_name, TypeDefValue *_typelist,
                  const std::string &_query) {
        type = GET;
        name = _name;
        typelist = _typelist;
        query = _query;
    }
    void init_upd(const std::string &_name, WordList *_wordlist) {
        type = UPD;
        name = _name;
        wordlist = _wordlist;
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
    void init(void) {
        index = false;
        query = false;
        likequery = false;
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
    FieldDef * fields;
    CustomGetUpdList * customs;
    TableDef(const std::string &_name)
        : name(_name)
    {
        next = NULL;
        fields = NULL;
        customs = NULL;
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

TableDef * parse_file(const std::string &fname);
void print_tokenized_file(const std::string &fname);
void print_tables(TableDef * tds);

#endif /* __PARSER_H__ */
