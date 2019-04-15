
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "parser.h"
#include PARSER_YY_HDR
#include "tokenizer.h"
#include "emit.h"

using namespace std;

void
usage(void)
{
    fprintf(stderr,
            "usage: sql3gen schema file.cc file.h file.proto\n");
}

int
main(int argc, char ** argv)
{
    if (argc != 5)
    {
        usage();
        return 1;
    }

    string schema_file(argv[1]);
    string cc_file(argv[2]);
    string h_file(argv[3]);
    string proto_file;
    string proto_h_file;

    if (getenv("DEBUG_TOKENIZER") != NULL)
    {
        print_tokenized_file(schema_file);
    }
    else
    {
        SchemaDef * schema = parse_file(schema_file);

        if (schema->package != "")
        {
            proto_file = argv[4];
            size_t filename_len = proto_file.size();
            if (filename_len < 7)
            {
                printf("invalid proto file name\n");
                return 1;
            }
            if (proto_file.compare (filename_len-6, string::npos,
                                    ".proto") != 0)
            {
                printf("proto file doesn't end in \".proto\"\n");
                return 1;
            }
            proto_h_file = proto_file.substr(0,filename_len-6) + ".pb.h";
        }
        else
        {
            fprintf(stderr,
                    "ERROR: schema does not contain PACKAGE\n");
            return 1;
        }

        struct stat sb;
        memset(&sb, 0, sizeof(sb));
        stat(schema_file.c_str(), &sb);
        struct tm schema_time;
        localtime_r(&sb.st_mtime, &schema_time);
        schema->schema_time.resize(100);
        schema->schema_time.resize(
            strftime((char*) schema->schema_time.c_str(), 100,
                     "%Y-%m%d-%H%M%S", &schema_time));

        print_tables(schema->tables);
        emit_source(cc_file, h_file, schema);
        emit_header(h_file, proto_h_file, schema);
        emit_proto(proto_file, schema);
        delete schema;
    }

    return 0;
}
