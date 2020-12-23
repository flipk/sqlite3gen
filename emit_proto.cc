
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
#include "template_proto.h"
#endif

using namespace std;

void
emit_proto(const std::string &fname, const SchemaDef *schema)
{
    ofstream  out(fname.c_str(), ios_base::out | ios_base::trunc);
    const TableDef *td;
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
    patterns["prototop_block"] = schema->prototop;

    output_PROTO_TOP(out, patterns);

    for (td = schema->tables; td; td = td->next)
    {
        const FieldDef * fd;
        bool use_table = false;
        for (fd = td->fields; fd; fd = fd->next)
            if (fd->attrs.protoid != -1)
                use_table = true;
        if (!use_table)
        {
            printf("table %s has no protoids, skipping\n",
                   td->name.c_str());
            continue;
        }

        string tablename = "TABLE_" + td->name;
        patterns["tablename"] = tablename;

        ostringstream protofields;
        ostringstream schema_version;

        schema_version << td->version;

        for (fd = td->fields; fd; fd = fd->next)
        {
            if (fd->attrs.protoid == -1)
            {
                printf("table %s field %s has no protoid, skipping\n",
                       td->name.c_str(), fd->name.c_str());
                continue;
            }
            printf("table %s field %s id %d\n",
                   td->name.c_str(),
                   fd->name.c_str(),
                   fd->attrs.protoid);

            patterns["fieldname"] = fd->name;

            if (fd->type.type == TYPE_SUBTABLE)
                patterns["optional_required"] = "/*SUBTABLE*/ repeated";
            else
                patterns["optional_required"] = "optional";

            switch (fd->type.type)
            {
            case TYPE_INT:
                patterns["fieldtype"] = "int32";
                break;
            case TYPE_INT64:
                patterns["fieldtype"] = "int64";
                break;
            case TYPE_DOUBLE:
                patterns["fieldtype"] = "double";
                break;
            case TYPE_TEXT:
                patterns["fieldtype"] = "string";
                break;
            case TYPE_BLOB:
                patterns["fieldtype"] = "bytes";
                break;
            case TYPE_BOOL:
                patterns["fieldtype"] = "bool";
                break;
            case TYPE_ENUM:
                patterns["fieldtype"] = fd->type.enum_name;
                break;
            case TYPE_SUBTABLE:
                patterns["fieldtype"] = string("TABLE_") + fd->name + "_m";
                break;
            }

            ostringstream fieldnum;
            fieldnum << fd->attrs.protoid;
            SET_PATTERN(fieldnum);

            output_PROTO_protofield(protofields, patterns);
        }

        SET_PATTERN(protofields);
        SET_PATTERN(schema_version);

        output_PROTO_message(out, patterns);
    }

    out << schema->protobottom;
}
