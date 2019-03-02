
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "emit.h"
#ifndef DEPENDING
#include "template_1.h"
#endif

using namespace std;

void emit_source(const std::string &fname,
                 const std::string &header_fname,
                 const SchemaDef *schema)
{
    ofstream  out(fname.c_str(), ios_base::out | ios_base::trunc);
    const TableDef *td;
    pattern_value_map  patterns;

    if (!out.good())
    {
        cerr << "can't open " << fname << ": " << strerror(errno) << endl;
        exit(1);
    }

    patterns["package"] = schema->package;
    patterns["headerfile"] = header_fname;
    patterns["sourcetop_block"] = schema->sourcetop;
    patterns["sourcebottom_block"] = schema->sourcebottom;

    output_SOURCE_TOP(out, patterns);

    ostringstream create_all_tables;
    ostringstream drop_all_tables;


    for (td = schema->tables; td; td = td->next)
    {
        ostringstream tableversion;
        ostringstream prepare_queries;
        ostringstream prepare_like_queries;
        ostringstream fieldnames;
        ostringstream questionmarks;
        ostringstream initial_values;
        ostringstream prepare_custom_get_queries;
        ostringstream prepare_custom_upd;
        ostringstream prepare_custom_del;
        ostringstream finalize_queries;
        ostringstream finalize_like_queries;
        ostringstream finalize_custom_get_queries;
        ostringstream finalize_custom_upd;
        ostringstream finalize_custom_del;
        ostringstream get_columns;
        ostringstream query_implementations;
        ostringstream query_like_implementations;
        ostringstream insert_binders;
        ostringstream update_binders;
        ostringstream custom_get_implementations;
        ostringstream custom_upd_implementations;
        ostringstream custom_del_implementations;
        ostringstream table_proto_copy_funcs;
        ostringstream proto_copy_to;
        ostringstream proto_copy_from;
        ostringstream table_create_fields;
        ostringstream index_creation;

        const FieldDef * fd;
        const CustomGetUpdList * cust;
        bool do_protobuf = false;

        patterns["tablename"] = td->name;
        tableversion << td->version;

        int column = 1;
        for (fd = td->fields; fd; fd = fd->next, column++)
        {
            fieldnames << fd->name;
            if (fd->next)
                fieldnames << ", ";
            if (fd->attrs.protoid != -1)
                do_protobuf = true;
        }

        // this needs to be defined before processing any
        // field getters, because we don't use "SELECT *"
        // anymore. every SELECT must specify all field names
        // to be future-proof against table changes.
        SET_PATTERN(fieldnames);

        column = 1;
        ostringstream column_index;
        for (fd = td->fields; fd; fd = fd->next, column++)
        {
            TypeDef t = fd->type.type;

            string fdnamelower = fd->name;
            for (size_t p = 0; p < fdnamelower.size(); p++)
                if (isupper(fdnamelower[p]))
                    fdnamelower[p] = tolower(fdnamelower[p]);

            patterns["fieldname"]          = fd->name;
            patterns["fieldname_lower"]    = fdnamelower;
            patterns["fieldtype"]          = TypeDef_to_Ctype(&fd->type, true);
            patterns["sqlite_column_func"] = TypeDef_to_sqlite_column(t);
            patterns["sqlite_bind_func"]   = TypeDef_to_sqlite_bind(t);
            patterns["sqlite_type"]        = TypeDef_to_sqlite_macro(t);

            column_index.str("");
            column_index << column;
            SET_PATTERN(column_index);

            questionmarks << "?";
            table_create_fields << fd->name << " "
                                << TypeDef_to_sqlite_create_type(t);
            if (fd->next)
            {
                fieldnames << ", ";
                questionmarks << ",";
                table_create_fields << ", ";
            }

            if (fd->attrs.query)
            {
                output_TABLE_prepare_query(prepare_queries, patterns);
                output_TABLE_finalize_query(finalize_queries, patterns);

                ostringstream query_bind;

                switch (t)
                {
                case TYPE_INT:
                case TYPE_INT64:
                case TYPE_DOUBLE:
                    output_TABLE_query_bind_pod(query_bind, patterns);
                    break;
                case TYPE_TEXT:
                case TYPE_BLOB:
                    output_TABLE_query_bind_string(query_bind, patterns);
                    break;
                case TYPE_BOOL:
                    output_TABLE_query_bind_bool(query_bind, patterns);
                    break;
                }

                SET_PATTERN(query_bind);

                output_TABLE_query_implementation(
                    query_implementations, patterns);
            }
            if (fd->attrs.likequery)
            {
                output_TABLE_prepare_like_query(
                    prepare_like_queries, patterns);
                output_TABLE_finalize_like_query(
                    finalize_like_queries, patterns);
                output_TABLE_query_like_implementation(
                    query_like_implementations, patterns);
            }
            if (fd->attrs.index)
            {
                output_TABLE_create_index(index_creation, patterns);
            }

            ostringstream initial_value;
            initial_value << "    " << fd->name;
            switch (t)
            {
            case TYPE_INT:
            case TYPE_INT64:
                initial_value
                    << " = " << fd->attrs.init_int << ";\n";
                break;
            case TYPE_DOUBLE:
                initial_value
                    << " = " << fd->attrs.init_double << ";\n";
                break;
            case TYPE_TEXT:
                initial_value
                    << " = \"" << fd->attrs.init_string << "\";\n";
                break;
            case TYPE_BLOB:
                initial_value << ".clear();\n";
                break;
            case TYPE_BOOL:
                initial_value
                    << " = "
                    << (fd->attrs.init_int ? "true" : "false")
                    << ";\n";
            }
            initial_values << initial_value.str();

            // reuse the initial value stuff for the protobuf
            // copyFrom method too (for when a field is not
            // populated in the Message).
            if (do_protobuf && fd->attrs.protoid != -1)
            {
                output_TABLE_proto_copy_to_field(
                    proto_copy_to, patterns);

                SET_PATTERN(initial_value);

                if (fd->type.type != TYPE_BOOL)
                    output_TABLE_proto_copy_from_field(
                        proto_copy_from, patterns);
                else
                    output_TABLE_proto_copy_from_field_bool(
                        proto_copy_from, patterns);
            }

            switch (t)
            {
            case TYPE_INT:
            case TYPE_INT64:
            case TYPE_DOUBLE:
                patterns["stmt"] = "insert";
                output_TABLE_insert_binder_pod(insert_binders, patterns);
                patterns["stmt"] = "update";
                output_TABLE_insert_binder_pod(update_binders, patterns);
                output_TABLE_get_column_pod(get_columns, patterns);
                break;
            case TYPE_TEXT:
            case TYPE_BLOB:
                patterns["stmt"] = "insert";
                output_TABLE_insert_binder_string(insert_binders, patterns);
                patterns["stmt"] = "update";
                output_TABLE_insert_binder_string(update_binders, patterns);
                output_TABLE_get_column_string(get_columns, patterns);
                break;
            case TYPE_BOOL:
                patterns["stmt"] = "insert";
                output_TABLE_insert_binder_bool(insert_binders, patterns);
                patterns["stmt"] = "update";
                output_TABLE_insert_binder_bool(update_binders, patterns);
                output_TABLE_get_column_bool(get_columns, patterns);
                break;
            }
        }

        // for ::update(), the last "?" is rowid, so the column_index
        // is one bigger than the last insert_binder.
        column_index.str("");
        column_index << column;
        SET_PATTERN(column_index);

        for (cust = td->customs; cust; cust = cust->next)
        {
            patterns["customname"] = cust->name;

            switch (cust->type)
            {
            case CustomGetUpdList::GET:
            {
                ostringstream custom_get_binders;

                patterns["querystring"] = cust->query;
                output_TABLE_prepare_custom_get_query(
                    prepare_custom_get_queries, patterns);
                output_TABLE_finalize_custom_get_query(
                    finalize_custom_get_queries, patterns);

                TypeDefValue *type;
                int count = 1;
                for (type = cust->typelist; type; type = type->next, count++)
                {
                    TypeDef t = type->type;
                    ostringstream arg_index;
                    arg_index << count;
                    SET_PATTERN(arg_index);
                    patterns["sqlite_bind_func"] = TypeDef_to_sqlite_bind(t);
                    switch (t)
                    {
                    case TYPE_INT:
                    case TYPE_BOOL:
                    case TYPE_INT64:
                    case TYPE_DOUBLE:
                        output_TABLE_custom_get_binder_pod(
                            custom_get_binders, patterns);
                        break;
                    case TYPE_TEXT:
                    case TYPE_BLOB:
                        output_TABLE_custom_get_binder_string(
                            custom_get_binders, patterns);
                        break;
                    }
                }

                SET_PATTERN(custom_get_binders);

                patterns["type_and_vX"] = make_custom_get_arglist(cust);

                output_TABLE_custom_get_implementation(
                    custom_get_implementations, patterns);
            }
            break;

            case CustomGetUpdList::UPD:
            {
                ostringstream custom_update_binders;
                ostringstream custom_fieldlist;
                ostringstream custom_questionmarks;

                output_TABLE_finalize_custom_upd(
                    finalize_custom_upd, patterns);

                int counter = 1;
                WordList * w;
                for (w = cust->wordlist; w; w = w->next, counter++)
                {
                    fd = find_field(td, w->word);

                    custom_fieldlist << w->word;
                    custom_questionmarks << "?";                    
                    if (w->next)
                    {
                        custom_fieldlist << ", ";
                        custom_questionmarks << ",";
                    }

                    TypeDef t = fd->type.type;
                    ostringstream fieldindex;
                    fieldindex << counter;
                    SET_PATTERN(fieldindex);
                    patterns["fieldname"] = fd->name;
                    patterns["sqlite_bind_func"] = TypeDef_to_sqlite_bind(t);

                    switch (t)
                    {
                    case TYPE_INT:
                    case TYPE_BOOL:
                    case TYPE_INT64:
                    case TYPE_DOUBLE:
                        output_TABLE_custom_upd_binder_pod(
                            custom_update_binders, patterns);
                        break;
                    case TYPE_TEXT:
                    case TYPE_BLOB:
                        output_TABLE_custom_upd_binder_string(
                            custom_update_binders, patterns);
                        break;
                    }
                }

                SET_PATTERN(custom_fieldlist);
                SET_PATTERN(custom_questionmarks);

                output_TABLE_prepare_custom_upd(
                    prepare_custom_upd, patterns);

                SET_PATTERN(custom_update_binders);

                ostringstream fieldindex;
                fieldindex << counter;
                SET_PATTERN(fieldindex);

                output_TABLE_custom_upd_implementation(
                    custom_upd_implementations, patterns);
            }
            break;

            case CustomGetUpdList::DEL:
            {
                ostringstream custom_del_binders;

                patterns["querystring"] = cust->query;
                output_TABLE_prepare_custom_del(
                    prepare_custom_del, patterns);
                output_TABLE_finalize_custom_del(
                    finalize_custom_del, patterns);

                TypeDefValue *type;
                int count = 1;
                for (type = cust->typelist; type; type = type->next, count++)
                {
                    TypeDef t = type->type;
                    ostringstream arg_index;
                    arg_index << count;
                    SET_PATTERN(arg_index);
                    patterns["sqlite_bind_func"] = TypeDef_to_sqlite_bind(t);
                    switch (t)
                    {
                    case TYPE_INT:
                    case TYPE_BOOL:
                    case TYPE_INT64:
                    case TYPE_DOUBLE:
                        output_TABLE_custom_del_binder_pod(
                            custom_del_binders, patterns);
                        break;
                    case TYPE_TEXT:
                    case TYPE_BLOB:
                        output_TABLE_custom_del_binder_string(
                            custom_del_binders, patterns);
                        break;
                    }
                }

                SET_PATTERN(custom_del_binders);

                patterns["type_and_vX"] = make_custom_get_arglist(cust);

                output_TABLE_custom_del_implementation(
                    custom_del_implementations, patterns);
            }
            break;
            }
        }

        if (do_protobuf && schema->package != "")
        {
            SET_PATTERN(proto_copy_to);
            SET_PATTERN(proto_copy_from);

            output_TABLE_proto_copy_funcs(
                table_proto_copy_funcs, patterns);
        }

        SET_PATTERN(tableversion);
        SET_PATTERN(prepare_queries);
        SET_PATTERN(prepare_like_queries);
        SET_PATTERN(questionmarks);
        SET_PATTERN(initial_values);
        SET_PATTERN(prepare_queries);
        SET_PATTERN(prepare_like_queries);
        SET_PATTERN(prepare_custom_get_queries);
        SET_PATTERN(prepare_custom_upd);
        SET_PATTERN(prepare_custom_del);
        SET_PATTERN(finalize_queries);
        SET_PATTERN(finalize_like_queries);
        SET_PATTERN(finalize_custom_get_queries);
        SET_PATTERN(finalize_custom_upd);
        SET_PATTERN(finalize_custom_del);
        SET_PATTERN(get_columns);
        SET_PATTERN(query_implementations);
        SET_PATTERN(query_like_implementations);
        SET_PATTERN(insert_binders);
        SET_PATTERN(update_binders);
        SET_PATTERN(custom_get_implementations);
        SET_PATTERN(custom_upd_implementations);
        SET_PATTERN(custom_del_implementations);
        SET_PATTERN(table_proto_copy_funcs);
        SET_PATTERN(table_create_fields);
        SET_PATTERN(index_creation);

        output_TABLE_CLASS_IMPL(out, patterns);

        output_CLASS_ALL_TABLES_create_a_table(create_all_tables, patterns);
        output_CLASS_ALL_TABLES_drop_a_table(drop_all_tables, patterns);
    }

    SET_PATTERN(create_all_tables);
    SET_PATTERN(drop_all_tables);

    output_CLASS_ALL_TABELS_IMPL(out, patterns);
}
