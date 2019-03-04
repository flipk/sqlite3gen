
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
    sqlite3 * pdb = NULL;

    sqlite3_vfs_aes::register_vfs();
    int r = sqlite3_open_v2("/tmp/test.db", &pdb,
                            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                            "aes");
    if (r != SQLITE_OK)
    {
        printf("open failed, r = %d\n", r);
        return 1;
    }
    test::SQL_TABLE_ALL_TABLES::init_all(pdb, &version_cb);
    test::SQL_TABLE_ids  t(pdb);

    std::ifstream  f("/tmp/test.txt");

    int count_good = 0;
    int count_bad = 0;
    while (f.good())
    {
        f >> t.id;
        if (t.get_by_id(t.id) == false)
            count_bad ++;
        else
            count_good ++;
    }
    printf("good %d bad %d\n", count_good, count_bad);

    sqlite3_close(pdb);
    return 0;
}
