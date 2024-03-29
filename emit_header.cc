
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
#include "template_header.h"
#endif

using namespace std;

void emit_header(const std::string &fname,
                 const std::string &proto_hdr_fname,
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

    out << "\n"
        << "///////////////////////////////////////////////////////////\n"
        << "// NOTE // NOTE // NOTE // NOTE // NOTE // NOTE // NOTE  //\n"
        << "//                                                       //\n"
        << "//       THIS FILE IS AUTOGENERATED BY sqlite3gen        //\n"
        << "// DO NOT EDIT THIS FILE, EDIT THE SOURCE AND REGENERATE //\n"
        << "//                                                       //\n"
        << "// NOTE // NOTE // NOTE // NOTE // NOTE // NOTE // NOTE  //\n"
        << "///////////////////////////////////////////////////////////\n";
    
    patterns["package"] = schema->package;
    patterns["headertop_block"] = schema->headertop;
    patterns["headerbottom_block"] = schema->headerbottom;

    ostringstream protobuf_header;
    ostringstream class_forwards;
    ostringstream defines;

    if (schema->option_xml)
        output_HEADER_define_xml(defines, patterns);

    if (schema->option_protobuf)
        output_HEADER_define_protobuf(defines, patterns);

    if (schema->package != "")
        protobuf_header << "#include \""
                        << proto_hdr_fname
                        << "\"";

    for (td = schema->tables; td; td = td->next)
    {
        class_forwards << "class SQL_TABLE_" << td->name << "; // forward\n";
    }

    SET_PATTERN(protobuf_header);
    SET_PATTERN(class_forwards);
    SET_PATTERN(defines);
    output_HEADER_TOP(out, patterns);

    for (td = schema->tables; td; td = td->next)
    {
        ostringstream tableversion;
        ostringstream stmt_by_decls;
        ostringstream stmt_by_decls_like;
        ostringstream stmt_custom_get_decls;
        ostringstream stmt_custom_upd_decls;
        ostringstream stmt_custom_del_decls;
        ostringstream table_field_type_name_decls;
        ostringstream table_field_type_name_tostring_decls;
        ostringstream table_query_method_protos;
        ostringstream table_query_like_method_protos;
        ostringstream table_custom_get_method_protos;
        ostringstream table_custom_upd_method_protos;
        ostringstream table_custom_del_method_protos;
        ostringstream table_proto_copy_protos;
        ostringstream xml_decoder_protos;

        const FieldDef *fd;
        const CustomGetUpdList * cust;
        bool include_protos = false;

        patterns["tablename"] = td->name;
        tableversion << td->version;

        for (fd = td->fields; fd; fd = fd->next)
        {
            patterns["fieldname"] = fd->name;
            if (fd->attrs.query)
            {
                patterns["fieldtype"] =
                    TypeDef_to_Ctype(&fd->type, true, fd->name);
                output_TABLE_CLASS_stmt_by_decl(
                    stmt_by_decls, patterns);
                output_TABLE_CLASS_table_query_method_protos(
                    table_query_method_protos, patterns);
            }
            if (fd->attrs.likequery)
            {
                output_TABLE_CLASS_stmt_by_decl_like(
                    stmt_by_decls_like, patterns);
                output_TABLE_CLASS_table_query_method_protos_like(
                    table_query_like_method_protos, patterns);
            }

            if (fd->type.type == TYPE_SUBTABLE)
            {
                patterns["foreign_key"] = fd->attrs.subtable_field->name;
                output_TABLE_CLASS_table_get_subtable_proto(
                    table_query_method_protos, patterns);
            }

            patterns["fieldtype"] =
                TypeDef_to_Ctype(&fd->type, false, fd->name);
            output_TABLE_CLASS_table_field_type_name_decls(
                table_field_type_name_decls, patterns);

            bool do_initial_value = true;
            ostringstream initial_value;
            switch (fd->type.type)
            {
            case TYPE_INT:
            case TYPE_INT64:
                initial_value << "= ";
                if (fd->attrs.init_string.length() != 0)
                    // the user used a macro here.
                    initial_value << fd->attrs.init_string;
                else
                    // try the integer.
                    initial_value << fd->attrs.init_int;
                break;
            case TYPE_DOUBLE:
                initial_value << "/* value is in the cc file */";
                break;
            case TYPE_TEXT:
                initial_value << "/* value is in the cc file */";
                break;
            case TYPE_BOOL:
                initial_value << "= ";
                if (fd->attrs.init_string.length() != 0)
                    // the user used a macro here.
                    initial_value << fd->attrs.init_string;
                else
                    // try the integer.
                    initial_value << (fd->attrs.init_int ? "true" : "false");
                break;
            case TYPE_ENUM:
                initial_value << "= ";
                initial_value << Dots_to_Colons(fd->attrs.init_string);
                break;
            case TYPE_SUBTABLE:
            case TYPE_BLOB:
                do_initial_value = false;
                break;
            }
            if (do_initial_value)
            {
                SET_PATTERN(initial_value);
                output_TABLE_CLASS_table_field_default_const_decls(
                    table_field_type_name_decls, patterns);
            }

            output_TABLE_CLASS_table_field_type_name_tostring_decls(
                table_field_type_name_tostring_decls, patterns);

            if (fd->attrs.protoid != -1)
                include_protos = true;

            output_TABLE_CLASS_xml_decoder_proto(
                xml_decoder_protos, patterns);
        }

        for (cust = td->customs; cust; cust = cust->next)
        {
            patterns["customname"] = cust->name;
            switch (cust->type)
            {
            case CustomGetUpdList::GET:
                output_TABLE_CLASS_stmt_custom_get_decl(
                    stmt_custom_get_decls, patterns);

                patterns["type_and_vX"] = make_custom_get_arglist(cust->typelist);

                table_custom_get_method_protos
                    << "// WHERE " << cust->query << "\n";

                output_TABLE_CLASS_table_custom_get_method_protos(
                    table_custom_get_method_protos, patterns);

                break;

            case CustomGetUpdList::UPD:
                output_TABLE_CLASS_stmt_custom_upd_decl(
                    stmt_custom_upd_decls, patterns);
                output_TABLE_CLASS_table_custom_upd_method_protos(
                    table_custom_upd_method_protos, patterns);

                break;

            case CustomGetUpdList::UPDBY:
                output_TABLE_CLASS_stmt_custom_updby_decl(
                    stmt_custom_upd_decls, patterns);

                patterns["type_and_vX"] = make_custom_get_arglist(cust->typelist);

                table_custom_upd_method_protos
                    << "// WHERE " << cust->query << "\n";

                output_TABLE_CLASS_table_custom_updby_method_protos(
                    table_custom_upd_method_protos, patterns);

                break;

            case CustomGetUpdList::DEL:
                output_TABLE_CLASS_stmt_custom_del_decl(
                    stmt_custom_del_decls, patterns);

                patterns["type_and_vX"] = make_custom_get_arglist(cust->typelist);

                table_custom_del_method_protos
                    << "// WHERE " << cust->query << "\n";

                output_TABLE_CLASS_table_custom_del_method_protos(
                    table_custom_del_method_protos, patterns);
                break;
            }
        }

        if (include_protos)
        {
            output_TABLE_CLASS_proto_copy_proto(
                table_proto_copy_protos, patterns);
        }
        else
            table_proto_copy_protos
                << "   // no proto IDs for this table\n";

        SET_PATTERN(tableversion);
        SET_PATTERN(stmt_by_decls);
        SET_PATTERN(stmt_by_decls_like);
        SET_PATTERN(stmt_custom_get_decls);
        SET_PATTERN(stmt_custom_upd_decls);
        SET_PATTERN(stmt_custom_del_decls);
        SET_PATTERN(table_field_type_name_decls);
        SET_PATTERN(table_field_type_name_tostring_decls);
        SET_PATTERN(table_query_method_protos);
        SET_PATTERN(table_query_like_method_protos);
        SET_PATTERN(table_custom_get_method_protos);
        SET_PATTERN(table_custom_upd_method_protos);
        SET_PATTERN(table_custom_del_method_protos);
        SET_PATTERN(table_proto_copy_protos);
        SET_PATTERN(xml_decoder_protos);

        output_TABLE_CLASS_DEFN(out, patterns);
    }

    for (csel = schema->custom_selects; csel; csel = csel->next)
    {
        ostringstream queryargs;
        ostringstream queryfields;
        ostringstream queryfields_tostring;

        patterns["queryname"] = csel->name;

        const TypeDefValue *tdv;
        int argcount = 1;
        for (tdv = csel->types; tdv; tdv = tdv->next)
        {
            queryargs << TypeDef_to_Ctype(tdv, true)
                      << " v" << argcount;
            argcount ++;
            if (tdv->next)
                queryargs << ", ";
        }

        for (size_t ind = 0; ind < csel->field_ptrs.size(); ind++)
        {
            TableDef * td = csel->field_table_ptrs[ind];
            FieldDef * fd = csel->field_ptrs[ind];
            const string fieldname = (fd == NULL) ? "rowid" : fd->name;
            const string fieldtype =
                (fd == NULL) ? "sqlite3_int64" :
                TypeDef_to_Ctype(&fd->type, false, fieldname);

            patterns["fieldtype"] = fieldtype;
            patterns["tablename"] = td->name;
            patterns["fieldname"] = fieldname;

            output_QUERY_CLASS_query_field(
                queryfields, patterns);
            output_QUERY_CLASS_table_field_type_name_tostring_decls(
                queryfields_tostring, patterns);
        }

        if (csel->where_clause.size() > 0)
            patterns["querybody"] = "WHERE " + csel->where_clause;
        else
            patterns["querybody"] = csel->full_statement;

        SET_PATTERN(queryargs);
        SET_PATTERN(queryfields);
        SET_PATTERN(queryfields_tostring);

        output_QUERY_CLASS_DEFN(out, patterns);
    }

    output_CLASS_ALL_TABELS_DEFN(out, patterns);
    output_HEADER_BOTTOM(out, patterns);
}
