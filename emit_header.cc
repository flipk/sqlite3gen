
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

void emit_header(const std::string &fname, const TableDef *tds)
{
    ofstream  out(fname.c_str(), ios_base::out | ios_base::trunc);
    const TableDef *td;
    pattern_value_map  patterns;

    if (!out.good())
    {
        cerr << "can't open " << fname << ": " << strerror(errno) << endl;
        exit(1);
    }
    
    output_HEADER_TOP(out, patterns);

    for (td = tds; td; td = td->next)
    {
        ostringstream stmt_by_decls;
        ostringstream stmt_by_decls_like;
        ostringstream stmt_custom_get_decls;
        ostringstream stmt_custom_upd_decls;
        ostringstream stmt_custom_del_decls;
        ostringstream table_field_type_name_decls;
        ostringstream table_query_method_protos;
        ostringstream table_query_like_method_protos;
        ostringstream table_custom_get_method_protos;
        ostringstream table_custom_upd_method_protos;
        ostringstream table_custom_del_method_protos;
        const FieldDef *fd;
        const CustomGetUpdList * cust;

        patterns["tablename"] = td->name;

        for (fd = td->fields; fd; fd = fd->next)
        {
            patterns["fieldname"] = fd->name;
            if (fd->attrs.query)
            {
                patterns["fieldtype"] =
                    TypeDef_to_Ctype(fd->type.type, true);
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
            patterns["fieldtype"] =
                TypeDef_to_Ctype(fd->type.type, false);
            output_TABLE_CLASS_table_field_type_name_decls(
                table_field_type_name_decls, patterns);
        }

        for (cust = td->customs; cust; cust = cust->next)
        {
            patterns["customname"] = cust->name;
            switch (cust->type)
            {
            case CustomGetUpdList::GET:
                output_TABLE_CLASS_stmt_custom_get_decl(
                    stmt_custom_get_decls, patterns);

                patterns["type_and_vX"] =
                    make_custom_get_arglist(cust);

                output_TABLE_CLASS_table_custom_get_method_protos(
                    table_custom_get_method_protos, patterns);

                break;

            case CustomGetUpdList::UPD:
                output_TABLE_CLASS_stmt_custom_upd_decl(
                    stmt_custom_upd_decls, patterns);
                output_TABLE_CLASS_table_custom_upd_method_protos(
                    table_custom_upd_method_protos, patterns);

                break;

            case CustomGetUpdList::DEL:
                output_TABLE_CLASS_stmt_custom_del_decl(
                    stmt_custom_del_decls, patterns);

                patterns["type_and_vX"] =
                    make_custom_get_arglist(cust);

                output_TABLE_CLASS_table_custom_del_method_protos(
                    table_custom_del_method_protos, patterns);
                break;
            }
        }

        SET_PATTERN(stmt_by_decls);
        SET_PATTERN(stmt_by_decls_like);
        SET_PATTERN(stmt_custom_get_decls);
        SET_PATTERN(stmt_custom_upd_decls);
        SET_PATTERN(stmt_custom_del_decls);
        SET_PATTERN(table_field_type_name_decls);
        SET_PATTERN(table_query_method_protos);
        SET_PATTERN(table_query_like_method_protos);
        SET_PATTERN(table_custom_get_method_protos);
        SET_PATTERN(table_custom_upd_method_protos);
        SET_PATTERN(table_custom_del_method_protos);

        output_TABLE_CLASS_DEFN(out, patterns);
    }

    output_CLASS_ALL_TABELS_DEFN(out, patterns);
}
