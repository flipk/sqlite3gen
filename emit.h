/* -*- Mode:c++; eval:(c-set-style "BSD"); c-basic-offset:4; indent-tabs-mode:nil; tab-width:8 -*- */

#include <string>
#include <sstream>

#include "parser.h"

void emit_header(const std::string &fname,
                 const std::string &proto_hdr_fname,
                 const SchemaDef *schema);

void emit_source(const std::string &fname,
                 const std::string &header_fname,
                 const SchemaDef *schema);

void emit_proto(const std::string &fname,
                const SchemaDef *schema);

#define SET_PATTERN(x) patterns[#x] = x.str();

static inline std::string
Dots_to_Colons(const std::string &in)
{
    std::string out;
    for (size_t ind = 0; ind < in.size(); ind++)
        if (in[ind] == '.')
            out += "::";
        else
            out += in[ind];
    return out;
}

static inline std::string
TypeDef_to_Ctype(const TypeDefValue *t, bool do_const,
                 const std::string &fieldname = "")
{
    switch (t->type)
    {
    case TYPE_INT:     return "int32_t";
    case TYPE_INT64:   return "int64_t";
    case TYPE_DOUBLE:  return "double";
    case TYPE_BOOL:    return "bool";
    case TYPE_TEXT:
    case TYPE_BLOB:
        if (do_const)
            return "const std::string &";
        // else
        return "std::string";
    case TYPE_ENUM:   return Dots_to_Colons(t->enum_name);
    case TYPE_SUBTABLE:
        return std::string("// NOTE this is only populated by get_subtable_")
            + fieldname + "()\n    std::vector<SQL_TABLE_"
            + fieldname + ">";
    }
    return "UNKNOWN_TYPE";
}

static inline std::string
TypeDef_to_string(TypeDef t)
{
    switch (t)
    {
    case TYPE_INT:      return "TYPE_INT";
    case TYPE_BOOL:     return "TYPE_BOOL";
    case TYPE_INT64:    return "TYPE_INT64";
    case TYPE_DOUBLE:   return "TYPE_DOUBLE";
    case TYPE_TEXT:     return "TYPE_TEXT";
    case TYPE_BLOB:     return "TYPE_BLOB";
    case TYPE_ENUM:     return "TYPE_ENUM";
    case TYPE_SUBTABLE: return "TYPE_SUBTABLE";
    }
    return "UNKNOWN_TYPE";
}

static inline std::string
TypeDef_to_sqlite_macro(TypeDef t)
{
    switch (t)
    {
    case TYPE_INT:     return "INTEGER";
    case TYPE_BOOL:    return "INTEGER";
    case TYPE_INT64:   return "INTEGER";
    case TYPE_DOUBLE:  return "FLOAT";
    case TYPE_TEXT:    return "TEXT";
    case TYPE_BLOB:    return "BLOB";
    case TYPE_ENUM:    return "INTEGER";
    case TYPE_SUBTABLE:
        fprintf(stderr, "ERROR: TypeDef_to_sqlite_macro "
                "TYPE_SUBTABLE invalid\n");
        exit(1);
    }
    return "UNKNOWN_TYPE";
}

static inline std::string
TypeDef_to_sqlite_type(TypeDef t)
{
    switch (t)
    {
    case TYPE_INT:     return "int";
    case TYPE_BOOL:    return "int";
    case TYPE_INT64:   return "int64";
    case TYPE_DOUBLE:  return "double";
    case TYPE_TEXT:    return "text";
    case TYPE_BLOB:    return "blob";
    case TYPE_ENUM:    return "int";
    case TYPE_SUBTABLE:
        fprintf(stderr, "ERROR: TypeDef_to_sqlite_type "
                "TYPE_SUBTABLE invalid\n");
        exit(1);
    }
    return "UNKNOWN_TYPE";
}

static inline std::string
TypeDef_to_sqlite_create_type(TypeDef t)
{
    switch (t)
    {
    case TYPE_INT:     return "integer";
    case TYPE_INT64:   return "int64";
    case TYPE_DOUBLE:  return "double";
    case TYPE_TEXT:    return "text";
    case TYPE_BLOB:    return "blob";
    case TYPE_BOOL:    return "integer";
    case TYPE_ENUM:    return "integer";
    case TYPE_SUBTABLE:
        fprintf(stderr, "ERROR: TypeDef_to_sqlite_create_type "
                "TYPE_SUBTABLE invalid\n");
        exit(1);
    }
    return "UNKNOWN_TYPE";
}

static inline std::string
TypeDef_to_sqlite_bind(TypeDef t)
{
    return std::string("sqlite3_bind_") + TypeDef_to_sqlite_type(t);
}

static inline std::string
TypeDef_to_sqlite_column(TypeDef t)
{
    return std::string("sqlite3_column_") + TypeDef_to_sqlite_type(t);
}

static inline std::string
make_custom_get_arglist(const TypeDefValue *typelist)
{
    std::ostringstream arglist;
    if (typelist == NULL)
        arglist << "void";
    else
    {
        const TypeDefValue *type;
        int count = 1;
        for (type = typelist;
             type;
             type = type->next, count++)
        {
            arglist << TypeDef_to_Ctype(type, true);
            arglist << " v";
            arglist << count;
            if (type->next)
                arglist << ", ";
        }
    }
    return arglist.str();
}

static inline FieldDef *
find_field(const TableDef *td, const std::string &name)
{
    FieldDef * f;
    for (f = td->fields; f; f = f->next)
        if (f->name == name)
            return f;
    return NULL;
}
