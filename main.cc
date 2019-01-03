
#include <stdio.h>
#include <string>

#include "parser.h"
#include PARSER_YY_HDR
#include "tokenizer.h"
#include "emit.h"

int
main(int argc, char ** argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "usage: sql3gen schema file.cc file.h\n");
        return 1;
    }

#if 0
    print_tokenized_file(argv[1]);
#else
    TableDef * tds = parse_file(argv[1]);
    print_tables(tds);
    emit_source(argv[2], argv[3], tds);
    emit_header(argv[3], tds);
    delete tds;
#endif
    return 0;
}
