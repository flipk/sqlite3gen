
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
#include "template_source.h"
#endif

using namespace std;

void emit_source(const std::string &fname,
                 const std::string &header_fname,
                 const SchemaDef *schema)
{
    ofstream  out(fname.c_str(), ios_base::out | ios_base::trunc);
    const TableDef *td;
    const CustomSelect *csel;
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
    ostringstream register_all_logfuncs;
    ostringstream table_export_all;
    ostringstream table_import_all;

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
        ostringstream table_xml_copy_funcs;
        ostringstream xml_copy_to;
        ostringstream xml_decoder_functions;
        ostringstream xml_decoder_initializers;
        ostringstream table_create_fields;
        ostringstream table_create_constraints;
        ostringstream index_creation;
        ostringstream get_subtable_implementations;
        ostringstream field_copies;
        ostringstream get_all_subtables;
        ostringstream insert_all_subtables;

        const FieldDef * fd;
        const CustomGetUpdList * cust;
        bool do_protobuf = false;

        patterns["tablename"] = td->name;
        tableversion << td->version;

        int column = 1;
        bool first_field = true;
        for (fd = td->fields; fd; fd = fd->next)
        {
            if (fd->type.type == TYPE_SUBTABLE)
                continue;

            if (!first_field)
                fieldnames << ", ";
            first_field = false;
            fieldnames << fd->name;
            if (fd->attrs.protoid != -1)
                do_protobuf = true;
            column++;
        }

        // this needs to be defined before processing any
        // field getters, because we don't use "SELECT *"
        // anymore. every SELECT must specify all field names
        // to be future-proof against table changes.
        SET_PATTERN(fieldnames);

        column = 1;
        ostringstream column_index;
        first_field = true;
        for (fd = td->fields; fd; fd = fd->next)
        {
            TypeDef t = fd->type.type;

            string fdnamelower = fd->name;
            for (size_t p = 0; p < fdnamelower.size(); p++)
                if (isupper(fdnamelower[p]))
                    fdnamelower[p] = tolower(fdnamelower[p]);

            patterns["fieldname"]          = fd->name;
            patterns["fieldname_lower"]    = fdnamelower;
            patterns["fieldtype"]          =
                TypeDef_to_Ctype(&fd->type, true, fd->name);

            ostringstream initial_value;
            initial_value << "    " << fd->name;
            switch (t)
            {
            case TYPE_INT:
            case TYPE_INT64:
                initial_value
                    << " = " << fd->attrs.init_int << ";\n";
                output_TABLE_copy_pod_to_xml(xml_copy_to, patterns);
                break;
            case TYPE_DOUBLE:
                initial_value
                    << " = " << fd->attrs.init_double << ";\n";
                output_TABLE_copy_pod_to_xml(xml_copy_to, patterns);
                break;
            case TYPE_TEXT:
                initial_value
                    << " = \"" << fd->attrs.init_string << "\";\n";
                output_TABLE_copy_string_to_xml(xml_copy_to, patterns);
                break;
            case TYPE_BLOB:
                initial_value << ".clear();\n";
                output_TABLE_copy_blob_to_xml(xml_copy_to, patterns);
                break;
            case TYPE_BOOL:
                initial_value
                    << " = "
                    << (fd->attrs.init_int ? "true" : "false")
                    << ";\n";
                output_TABLE_copy_bool_to_xml(xml_copy_to, patterns);
                break;
            case TYPE_ENUM:
                initial_value
                    << " = "
                    << Dots_to_Colons(fd->attrs.init_string)
                    << ";\n";
                output_TABLE_copy_enum_to_xml(xml_copy_to, patterns);
                break;
            case TYPE_SUBTABLE:
                initial_value << ".clear();\n";
                output_TABLE_copy_subtable_to_xml(xml_copy_to, patterns);
                output_TABLE_CLASS_get_all_subtables_one(
                    get_all_subtables, patterns);
                output_TABLE_CLASS_insert_all_subtables_one(
                    insert_all_subtables, patterns);
                break;
            }
            initial_values << initial_value.str();
            SET_PATTERN(initial_value);

            switch (t)
            {
            case TYPE_INT:
            case TYPE_INT64:
                output_TABLE_copy_xml_to_pod(
                    xml_decoder_functions, patterns);
                break;
            case TYPE_DOUBLE:
                output_TABLE_copy_xml_to_pod(
                    xml_decoder_functions, patterns);
                break;
            case TYPE_TEXT:
                output_TABLE_copy_xml_to_string(
                    xml_decoder_functions, patterns);
                break;
            case TYPE_BLOB:
                output_TABLE_copy_xml_to_blob(
                    xml_decoder_functions, patterns);
                break;
            case TYPE_BOOL:
                output_TABLE_copy_xml_to_bool(
                    xml_decoder_functions, patterns);
                break;
            case TYPE_ENUM:
                output_TABLE_copy_xml_to_enum(
                    xml_decoder_functions, patterns);
                break;
            case TYPE_SUBTABLE:
                output_TABLE_copy_xml_to_subtable(
                    xml_decoder_functions, patterns);
                break;
            }
            output_TABLE_xml_decoder_initializer(
                xml_decoder_initializers, patterns);

            field_copies << "    " << fd->name
                         << " = other." << fd->name
                         << ";\n";

            if (t == TYPE_SUBTABLE)
            {
                patterns["this_key"] =
                    fd->attrs.subtable_field->attrs.foreign_field;
                patterns["other_key"] = fd->attrs.subtable_field->name;
                output_TABLE_get_subtable_implementation(
                    get_subtable_implementations, patterns);
                output_TABLE_proto_copy_to_subtable(
                    proto_copy_to, patterns);
                output_TABLE_proto_copy_from_subtable(
                    proto_copy_from, patterns);
                continue;
            }

            patterns["sqlite_column_func"] = TypeDef_to_sqlite_column(t);
            patterns["sqlite_bind_func"]   = TypeDef_to_sqlite_bind(t);
            patterns["sqlite_type"]        = TypeDef_to_sqlite_macro(t);

            column_index.str("");
            column_index << column;
            SET_PATTERN(column_index);

            if (!first_field)
            {
                questionmarks << ",";
                table_create_fields << ", ";
            }
            first_field = false;
            questionmarks << "?";
            table_create_fields << fd->name << " "
                                << TypeDef_to_sqlite_create_type(t);
            if (fd->attrs.notnull)
                table_create_fields << " NOT NULL";
            if (fd->attrs.unique)
                table_create_fields << " UNIQUE";

            if (fd->attrs.foreign)
            {
                patterns["foreign_table"] = fd->attrs.foreign_table;
                patterns["foreign_field"] = fd->attrs.foreign_field;

                output_TABLE_create_constraints(
                    table_create_constraints, patterns);
            }

            if (fd->attrs.query)
            {
                output_TABLE_prepare_query(prepare_queries, patterns);
                output_TABLE_finalize_query(finalize_queries, patterns);

                ostringstream query_bind;

                switch (t)
                {
                case TYPE_INT:
                case TYPE_ENUM:
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
                case TYPE_SUBTABLE:
                    fprintf(stderr, "ERROR: query SUBTABLE "
                            "shouldn't get here\n");
                    exit(1);
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

            // reuse the initial value stuff for the protobuf
            // copyFrom method too (for when a field is not
            // populated in the Message).
            if (do_protobuf && fd->attrs.protoid != -1)
            {
                switch (fd->type.type)
                {
                case TYPE_INT:
                case TYPE_INT64:
                case TYPE_TEXT:
                case TYPE_BLOB:
                case TYPE_DOUBLE:
                    output_TABLE_proto_copy_to_field(
                        proto_copy_to, patterns);
                    break;
                case TYPE_BOOL:
                    output_TABLE_proto_copy_to_field_bool(
                        proto_copy_to, patterns);
                    break;
                case TYPE_ENUM:
                    output_TABLE_proto_copy_to_field_enum(
                        proto_copy_to, patterns);
                    break;
                case TYPE_SUBTABLE:
                    // handled above, this is a NOTREACHED
                    break;
                }

                switch (fd->type.type)
                {
                case TYPE_INT:
                case TYPE_INT64:
                case TYPE_TEXT:
                case TYPE_BLOB:
                case TYPE_DOUBLE:
                case TYPE_ENUM:
                    output_TABLE_proto_copy_from_field(
                        proto_copy_from, patterns);
                    break;
                case TYPE_BOOL:
                    output_TABLE_proto_copy_from_field_bool(
                        proto_copy_from, patterns);
                    break;
                case TYPE_SUBTABLE:
                    // handled above, this is a NOTREACHED
                    break;
                }
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
            case TYPE_ENUM:
                patterns["stmt"] = "insert";
                output_TABLE_insert_binder_enum(insert_binders, patterns);
                patterns["stmt"] = "update";
                output_TABLE_insert_binder_enum(update_binders, patterns);
                output_TABLE_get_column_enum(get_columns, patterns);
                break;
            case TYPE_SUBTABLE:
                // nothing. user expected to get/update/insert subtables
                // using the classes for those tables.
                break;
            }

            column++;
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
                    case TYPE_BOOL:
                        output_TABLE_custom_get_binder_bool(
                            custom_get_binders, patterns);
                        break;
                    case TYPE_ENUM:
                        output_TABLE_custom_get_binder_enum(
                            custom_get_binders, patterns);
                        break;
                    case TYPE_SUBTABLE:
                        fprintf(stderr, "ERROR: subtable custom get binder "
                                "should be invalid\n");
                        exit(1);
                    }
                }

                SET_PATTERN(custom_get_binders);

                patterns["type_and_vX"] = make_custom_get_arglist(cust);

                output_TABLE_custom_get_implementation(
                    custom_get_implementations, patterns);
            }
            break;

            case CustomGetUpdList::UPD:
            case CustomGetUpdList::UPDBY:
            {
                ostringstream custom_update_binders;
                ostringstream custom_fieldlist;
                ostringstream custom_questionmarks;
                bool updby = (cust->type == CustomGetUpdList::UPDBY);

                if (updby)
                    output_TABLE_finalize_custom_updby(
                        finalize_custom_upd, patterns);
                else
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

                    if (updby)
                        patterns["by"] = "update_by";
                    else
                        patterns["by"] = "update";

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
                    case TYPE_BOOL:
                        output_TABLE_custom_upd_binder_bool(
                            custom_update_binders, patterns);
                        break;
                    case TYPE_ENUM:
                        output_TABLE_custom_upd_binder_enum(
                            custom_update_binders, patterns);
                        break;
                    case TYPE_SUBTABLE:
                        fprintf(stderr, "ERROR: subtable custom upd binder "
                                "should be invalid\n");
                        exit(1);
                    }
                }
                // note counter continues updating on from
                // this value, below, in the case of a
                // custom-updby.

                SET_PATTERN(custom_fieldlist);
                SET_PATTERN(custom_questionmarks);

                if (updby)
                    output_TABLE_prepare_custom_updby(
                        prepare_custom_upd, patterns);
                else
                    output_TABLE_prepare_custom_upd(
                        prepare_custom_upd, patterns);

                ostringstream fieldindex;
                fieldindex << counter;
                SET_PATTERN(fieldindex);

                if (updby)
                {
                    patterns["type_and_vX"] = make_custom_get_arglist(cust);
                    patterns["querystring"] = cust->query;

                    TypeDefValue *type;
                    // note counter continues on from where it left
                    // off from the binders above, but fieldnumber
                    // starts at one.
                    int fieldnumber = 1;
                    for (type = cust->typelist;
                         type;
                         type = type->next, counter++, fieldnumber++)
                    {
                        TypeDef t = type->type;
                        ostringstream arg_index;
                        arg_index << counter;
                        SET_PATTERN(arg_index);

                        ostringstream fieldindex;
                        fieldindex << fieldnumber;
                        SET_PATTERN(fieldindex);

                        patterns["sqlite_bind_func"] =
                            TypeDef_to_sqlite_bind(t);
                        switch (t)
                        {
                        case TYPE_INT:
                        case TYPE_INT64:
                        case TYPE_DOUBLE:
                            output_TABLE_custom_updby_binder_pod(
                                custom_update_binders, patterns);
                            break;
                        case TYPE_TEXT:
                        case TYPE_BLOB:
                            output_TABLE_custom_updby_binder_string(
                                custom_update_binders, patterns);
                            break;
                        case TYPE_BOOL:
                            output_TABLE_custom_updby_binder_bool(
                                custom_update_binders, patterns);
                            break;
                        case TYPE_ENUM:
                            output_TABLE_custom_updby_binder_enum(
                                custom_update_binders, patterns);
                            break;
                        case TYPE_SUBTABLE:
                            fprintf(stderr, "ERROR: subtable custom "
                                    "upd binder should be invalid\n");
                            exit(1);
                        }
                    }
                }

                SET_PATTERN(custom_update_binders);

                if (updby)
                    output_TABLE_custom_updby_implementation(
                        custom_upd_implementations, patterns);
                else
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
                    case TYPE_BOOL:
                        output_TABLE_custom_del_binder_bool(
                            custom_del_binders, patterns);
                        break;
                    case TYPE_ENUM:
                        output_TABLE_custom_del_binder_enum(
                            custom_del_binders, patterns);
                        break;
                    case TYPE_SUBTABLE:
                        fprintf(stderr, "ERROR: subtable custom "
                                    "del binder should be invalid\n");
                        exit(1);
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

        SET_PATTERN(xml_copy_to);
        SET_PATTERN(xml_decoder_functions);
        SET_PATTERN(xml_decoder_initializers);
        output_TABLE_xml_copy_funcs(table_xml_copy_funcs, patterns);

        table_create_fields << table_create_constraints.str();

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
        SET_PATTERN(table_xml_copy_funcs);
        SET_PATTERN(table_create_fields);
        SET_PATTERN(index_creation);
        SET_PATTERN(get_subtable_implementations);
        SET_PATTERN(field_copies);
        SET_PATTERN(get_all_subtables);
        SET_PATTERN(insert_all_subtables);

        patterns["is_subtable"] = td->is_subtable ? "true" : "false";

        output_CLASS_ALL_TABLES_export_a_table(
            table_export_all, patterns);
        if (td->is_subtable == false)
        {
            output_CLASS_ALL_TABLES_import_a_table(
                table_import_all, patterns);
        }

        output_TABLE_CLASS_IMPL(out, patterns);

        output_CLASS_ALL_TABLES_create_a_table(create_all_tables, patterns);
        output_CLASS_ALL_TABLES_drop_a_table(drop_all_tables, patterns);
        output_CLASS_ALL_TABLES_register_a_logfunc(
            register_all_logfuncs, patterns);
    }

    for (csel = schema->custom_selects; csel; csel = csel->next)
    {
        const WordList *wl;
        const TypeDefValue *tdv;
        int argcount;

        ostringstream queryargs;
        ostringstream get_columns;
        ostringstream querybinders;
        ostringstream queryfields;
        ostringstream querytables;

        patterns["queryname"] = csel->name;

        argcount = 1;
        for (tdv = csel->types; tdv; tdv = tdv->next)
        {
            string fieldtype = TypeDef_to_Ctype(tdv, true);
            queryargs << fieldtype
                      << " v" << argcount;
            if (tdv->next)
                queryargs << ", ";

            ostringstream arg_index;
            arg_index << argcount;
            SET_PATTERN(arg_index);

            patterns["sqlite_bind_func"] = TypeDef_to_sqlite_bind(tdv->type);
            patterns["customname"] = "query";

            switch (tdv->type)
            {
            case TYPE_INT:
            case TYPE_INT64:
            case TYPE_DOUBLE:
                output_TABLE_custom_get_binder_pod(querybinders, patterns);
                break;
            case TYPE_TEXT:
            case TYPE_BLOB:
                output_TABLE_custom_get_binder_string(querybinders, patterns);
                break;
            case TYPE_ENUM:
                output_TABLE_custom_get_binder_enum(querybinders, patterns);
                break;
            case TYPE_BOOL:
                output_TABLE_custom_get_binder_bool(querybinders, patterns);
                break;
            case TYPE_SUBTABLE:
                fprintf(stderr, "ERROR: query SUBTABLE "
                        "shouldn't get here\n");
                exit(1);
            }

            argcount ++;
        }

        for (wl = csel->field_names; wl; wl = wl->next)
        {
            queryfields << wl->word;
            if (wl->next)
                queryfields << ", ";
        }

        argcount = 0;
        for (size_t ind = 0; ind < csel->field_ptrs.size(); ind++)
        {
            TableDef * td = csel->field_table_ptrs[ind];
            FieldDef * fd = csel->field_ptrs[ind];
            const string fieldname = (fd == NULL) ? "rowid" : fd->name;
            const string fieldtype =
                (fd == NULL) ? "sqlite3_int64" :
                TypeDef_to_Ctype(&fd->type, false, fieldname);

            TypeDef t = (fd == NULL) ? TYPE_INT64 : fd->type.type;

            ostringstream column_index;
            column_index << argcount;
            SET_PATTERN(column_index);

            patterns["sqlite_type"] = TypeDef_to_sqlite_macro(t);
            patterns["fieldname"] = td->name + "_" + fieldname;
            patterns["sqlite_column_func"] = TypeDef_to_sqlite_column(t);

            switch (t)
            {
            case TYPE_INT:
            case TYPE_INT64:
            case TYPE_DOUBLE:
                output_TABLE_get_column_pod(get_columns, patterns);
                break;
            case TYPE_TEXT:
            case TYPE_BLOB:
                output_TABLE_get_column_string(get_columns, patterns);
                break;
            case TYPE_BOOL:
                output_TABLE_get_column_bool(get_columns, patterns);
                break;
            case TYPE_ENUM:
            {
                ostringstream initial_value;
                initial_value << "    " << td->name << "_" << fd->name
                              << " = "
                              << Dots_to_Colons(fd->attrs.init_string)
                              << ";\n";
                SET_PATTERN(initial_value);
                patterns["fieldtype"]          =
                    TypeDef_to_Ctype(&fd->type, true, fd->name);
                output_TABLE_get_column_enum(get_columns, patterns);
                break;
            }
            case TYPE_SUBTABLE:
                // nothing, NOTREACHED
                break;
            }

            argcount++;
        }

        for (wl = csel->table_names; wl; wl = wl->next)
        {
            querytables << wl->word;
            if (wl->next)
                querytables << ", ";
        }

        patterns["querywhere"] = csel->where_clause;

        SET_PATTERN(queryfields);
        SET_PATTERN(querytables);
        SET_PATTERN(querybinders);
        SET_PATTERN(queryargs);
        SET_PATTERN(get_columns);
        output_QUERY_CLASS_IMPL(out, patterns);
        output_CLASS_SQL_QUERY_register_a_logfunc(
            register_all_logfuncs, patterns);
    }

    SET_PATTERN(create_all_tables);
    SET_PATTERN(drop_all_tables);
    SET_PATTERN(register_all_logfuncs);
    SET_PATTERN(table_export_all);
    SET_PATTERN(table_import_all);

    output_CLASS_ALL_TABELS_IMPL(out, patterns);
}
