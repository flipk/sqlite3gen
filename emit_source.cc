
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
                 const std::string &header_fname, const TableDef *tds)
{
    ofstream  out(fname.c_str(), ios_base::out | ios_base::trunc);
    const TableDef *td;
    pattern_value_map  patterns;

    if (!out.good())
    {
        cerr << "can't open " << fname << ": " << strerror(errno) << endl;
        exit(1);
    }

    output_SOURCE_TOP(out, patterns);

    ostringstream create_all_tables;

    for (td = tds; td; td = td->next)
    {
        ostringstream prepare_queries;
        ostringstream prepare_like_queries;
        ostringstream fieldnames;
        ostringstream questionmarks;
        ostringstream prepare_custom_get_queries;
        ostringstream prepare_custom_upd;
        ostringstream finalize_queries;
        ostringstream finalize_like_queries;
        ostringstream finalize_custom_get_queries;
        ostringstream finalize_custom_upd;
        ostringstream get_columns;
        ostringstream query_implementations;
        ostringstream query_like_implementations;
        ostringstream insert_binders;
        ostringstream custom_get_implementations;
        ostringstream custom_upd_implementations;
        ostringstream index_creation;

        const FieldDef * fd;
        const CustomGetUpdList * cust;

        patterns["tablename"] = td->name;

        int column = 1;
        ostringstream column_index;
        for (fd = td->fields; fd; fd = fd->next, column++)
        {
            TypeDef t = fd->type.type;

            patterns["fieldname"]          = fd->name;
            patterns["fieldtype"]          = TypeDef_to_Ctype(t, true);
            patterns["sqlite_column_func"] = TypeDef_to_sqlite_column(t);
            patterns["sqlite_bind_func"]   = TypeDef_to_sqlite_bind(t);
            patterns["sqlite_type"]        = TypeDef_to_sqlite_macro(t);

            column_index.str("");
            column_index << column;
            SET_PATTERN(column_index);

            fieldnames << fd->name;
            questionmarks << "?";
            if (fd->next)
            {
                fieldnames << ", ";
                questionmarks << ",";
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

            switch (t)
            {
            case TYPE_INT:
            case TYPE_INT64:
            case TYPE_DOUBLE:
                output_TABLE_insert_binder_pod(insert_binders, patterns);
                output_TABLE_get_column_pod(get_columns, patterns);
                break;
            case TYPE_TEXT:
            case TYPE_BLOB:
                output_TABLE_insert_binder_string(insert_binders, patterns);
                output_TABLE_get_column_string(get_columns, patterns);
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
            if (cust->type == CustomGetUpdList::GET)
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
            else if (cust->type == CustomGetUpdList::UPD)
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
        }

        SET_PATTERN(prepare_queries);
        SET_PATTERN(prepare_like_queries);
        SET_PATTERN(fieldnames);
        SET_PATTERN(questionmarks);
        SET_PATTERN(prepare_queries);
        SET_PATTERN(prepare_like_queries);
        SET_PATTERN(prepare_custom_get_queries);
        SET_PATTERN(prepare_custom_upd);
        SET_PATTERN(finalize_queries);
        SET_PATTERN(finalize_like_queries);
        SET_PATTERN(finalize_custom_get_queries);
        SET_PATTERN(finalize_custom_upd);
        SET_PATTERN(get_columns);
        SET_PATTERN(query_implementations);
        SET_PATTERN(query_like_implementations);
        SET_PATTERN(insert_binders);
        SET_PATTERN(custom_get_implementations);
        SET_PATTERN(custom_upd_implementations);
        SET_PATTERN(index_creation);

        output_TABLE_CLASS_IMPL(out, patterns);

        output_CLASS_ALL_TABLES_create_a_table(create_all_tables, patterns);
    }

    SET_PATTERN(create_all_tables);

    output_CLASS_ALL_TABELS_IMPL(out, patterns);
}
