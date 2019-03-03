
#include "aes_vfs.h"
#include "obj/schema.h"
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sstream>
#include <fstream>

void version_cb(const std::string &table_name,
                int version_in_file,
                int version_in_code)
{
    printf("table %s version from %d to %d\n",
           table_name.c_str(), version_in_file, version_in_code);
}

int
main()
{
    sqlite3 * pdb;

    srandom((unsigned int)getpid() * (unsigned int)time(NULL));

    sqlite3_vfs_aes::register_vfs();
    sqlite3_open("/tmp/test.db", &pdb);
    test::SQL_TABLE_ALL_TABLES::init_all(pdb, &version_cb);
    test::SQL_TABLE_ids  t(pdb);

    std::ofstream f("/tmp/test.txt");
    std::ostringstream n;
    for (int i = 0; i < 10000; i++)
    {
        t.id = random();
        n.str("");
        n << "N_" << t.id;
        t.name = n.str();
        t.insert();
        f << t.id << std::endl;
    }

    sqlite3_close(pdb);
    return 0;
}
