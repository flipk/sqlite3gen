
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

bool in_template = false;
string template_name;
string template_body;

#define BOUNDARY "________"
#define BOUNDARY_LEN 8

#ifndef ECHO_OUTPUT
#define ECHO_OUTPUT 0
#endif

vector<int> ats;
struct pattern_info {
    int start_pos;
    int end_pos;
    string  pattern;
};
vector<pattern_info> patterns;
map<string,string> unique_patterns;

string
escape(const string &in)
{
    string out;
    for (int ind = 0; ind < (int)in.length(); ind++)
    {
        if (in[ind] == '"')
        {
            out += "\\\"";
            continue;
        }
        else if (in[ind] == 10)
        {
            out += "\" << std::endl\n        << \"";
            continue;
        }
        else if (in[ind] == '\\')
        {
            out += "\\\\";
            continue;
        }
        // else
        out += in[ind];
    }
    return out;
}

void
add_template(ostream &cc_out, ostream &h_out,
             const string &name, const string &body)
{
    if (ECHO_OUTPUT)
        fprintf(stderr,
                "template '%s' of length %d\n",
                name.c_str(), (int) body.length());

    // now to identify all the @@patterns@@
    for (int ind = 0; ind < (int) body.length() - 2; ind++)
        if (body[ind] == '@' && body[ind+1] == '@')
            ats.push_back(ind++);

    if (ECHO_OUTPUT)
        for (int ind = 0; ind < (int) ats.size(); ind++)
            printf("ats[%d] = %d\n", ind, ats[ind]);

    for (int ind = 0; ind < (int) ats.size(); ind += 2)
    {
        pattern_info pi;
        pi.start_pos = ats[ind];
        pi.end_pos = ats[ind+1];
        pi.pattern = body.substr(pi.start_pos + 2,
                                 pi.end_pos - pi.start_pos - 2);
        unique_patterns[pi.pattern] = 1;
        patterns.push_back(pi);
    }

    if (ECHO_OUTPUT)
        for (int ind = 0; ind < (int) patterns.size(); ind++)
        {
            pattern_info &pi = patterns[ind];
            fprintf(stderr, "  pattern: [%d-%d] %s\n",
                    pi.start_pos, pi.end_pos, pi.pattern.c_str());
        }

    cc_out << "\n"
           << "void output_" << name
           << "(std::ostream &out, const pattern_value_map &pattern_values)\n"
           << "{\n";

    h_out << "// must define patterns:\n";

    map<string,string>::iterator it;
    for (it = unique_patterns.begin();
         it != unique_patterns.end();
         it++)
    {
        h_out << "//     " << it->first << "\n";
    }

    h_out << "void output_" << name << "(\n"
          << "    std::ostream &out, \n"
          << "    const pattern_value_map &pattern_values);\n"
          << "\n";

    string piece;
    int last_pos = 0;
    for (int ind = 0; ind < (int) patterns.size(); ind++)
    {
        pattern_info &pi = patterns[ind];
        piece = body.substr(last_pos, pi.start_pos - last_pos);
        cc_out << "    out << \"" << escape(piece) << "\";\n";
        cc_out << "    out << find_pattern(pattern_values, \""
               << pi.pattern << "\", \"" << name << "\");\n";
        last_pos = pi.end_pos + 2;
    }
    piece = body.substr(last_pos);
    cc_out << "    out << \"" << escape(piece) << "\";\n"
           << "}\n";

    ats.clear();
    patterns.clear();
    unique_patterns.clear();
}

void
process_line(ostream &cc_out, ostream &h_out,
             const string &line)
{
    if (in_template == false)
    {
        if (memcmp(line.c_str(), BOUNDARY, BOUNDARY_LEN) == 0)
        {
            in_template = true;
            template_body.clear();
            template_name = line.substr(BOUNDARY_LEN + 1);
        }
        return;
    }
    // else
    if (memcmp(line.c_str(), BOUNDARY, BOUNDARY_LEN) == 0)
    {
        in_template = false;
        add_template(cc_out, h_out, template_name, template_body);
        if (line.substr(BOUNDARY_LEN + 1, 3) != "END")
        {
            cerr << "ERROR 'END' != '"
                 << line.substr(BOUNDARY_LEN + 1)
                 << "'\n";
            exit(1);
        }
        return;
    }
    template_body += line;
    template_body += "\n";
}

void print_line(const string &line)
{
    fprintf(stderr, "length : %d\n", (int) line.length());
    unsigned char * ptr = (unsigned char *) line.c_str();
    fprintf(stderr, "contents: ");
    for (int ind = 0; ind < (int) line.length(); ind++)
        fprintf(stderr, "%02x ", ptr[ind]);
    fprintf(stderr, "\n");
}

int
main(int argc, char ** argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "usage: template_to_c templ file.cc file.h\n");
        return 1;
    }

    ifstream tmpl_in(argv[1]);
    ofstream cc_out(argv[2]);
    ofstream  h_out(argv[3]);

    cc_out << "\n"
           << "#include <iostream>\n"
           << "#include \"" << argv[3] << "\"\n"
           << "\n";

    h_out << "\n"
          << "#include <map>\n"
          << "#include <iostream>\n"
          << "#include \"../template_patterns.h\"\n"
          << "\n";

    string  line;
    while (1)
    {
        line.resize(300);
        tmpl_in.getline((char*)line.c_str(), line.length());
        if (!tmpl_in.good())
            break;
        line.resize(tmpl_in.gcount()-1);
        process_line(cc_out, h_out, line);
//        print_line(line);
    }

    return 0;
}
