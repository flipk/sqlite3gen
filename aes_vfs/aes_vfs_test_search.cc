
#include "aes_vfs.h"
#include "obj/schema.h"
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sstream>
#include <fstream>

void version_cb(sqlite3 *pdb,
                const std::string &table_name,
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

    AES_VFS::sqlite3_vfs_aes::setKey("SOME PASSWORD");
    AES_VFS::sqlite3_vfs_aes::register_vfs();
    int r = sqlite3_open_v2("/tmp/test.db", &pdb,
                            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                            "aes");
    if (r != SQLITE_OK)
    {
        printf("open failed, r = %d\n", r);
        return 1;
    }
    test::SQL_TABLE_ALL_TABLES::init_all(pdb, &version_cb);
    {
        test::SQL_TABLE_ids  t(pdb);

        std::ifstream  f("/tmp/test.txt");

        time_t last = time(NULL);
        int count_good = 0, last_good = 0;
        int count_bad = 0, last_bad = 0;
        while (f.good())
        {
            f >> t.id;
            if (t.get_by_id(t.id) == false)
                count_bad ++;
            else
                count_good ++;
            time_t now = time(NULL);
            if (now != last)
            {
                printf("good %d %d bad %d %d\n",
                       count_good - last_good, count_good,
                       count_bad - last_bad, count_bad);
                last = now;
                last_good = count_good;
                last_bad = count_bad;
            }
        }
        printf("good %d bad %d\n", count_good, count_bad);
    } // destructor on "t" to release locks.

    if (sqlite3_close(pdb) != SQLITE_OK)
        printf("ERR!  close returns %d\n", r);
    sqlite3_shutdown();

    return 0;
}
