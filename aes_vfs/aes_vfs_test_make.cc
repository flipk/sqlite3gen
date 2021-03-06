
#include "aes_vfs.h"
#include "obj/schema.h"
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sstream>
#include <fstream>

#define VERBOSE 0

void version_cb(sqlite3 *pdb,
                const std::string &table_name,
                int version_in_file,
                int version_in_code)
{
    printf("table %s version from %d to %d\n",
           table_name.c_str(), version_in_file, version_in_code);
}

static void log_upd(void *arg, sqlite3_stmt *stmt)
{
#if VERBOSE
    char * sql = sqlite3_expanded_sql(stmt);
    printf("SQL UPDATE: %s\n", sql);
    sqlite3_free(sql);
#endif
}

static void log_get(void *arg, sqlite3_stmt *stmt)
{
#if VERBOSE
    char * sql = sqlite3_expanded_sql(stmt);
    printf("SQL GET: %s\n", sql);
    sqlite3_free(sql);
#endif
}

static void log_err(void *arg, const std::string &msg)
{
    printf("SQL ERROR: %s\n", msg.c_str());
}

int
main()
{
    sqlite3 * pdb;

    srandom((unsigned int)getpid() * (unsigned int)time(NULL));

    unlink("/tmp/test.db");
    unlink("/tmp/test.db-journal");
    unlink("/tmp/test.text");

    test::SQL_TABLE_ids::register_log_funcs(
        &log_upd, &log_get, NULL, &log_err, NULL);
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

    sqlite3_exec(pdb, "pragma journal_mode=off",
                 NULL, NULL, NULL);
    sqlite3_exec(pdb, "pragma synchronous=0",
                 NULL, NULL, NULL);

    test::SQL_TABLE_ALL_TABLES::init_all(pdb, &version_cb);
    {
        test::SQL_TABLE_ids  t(pdb);

        std::ofstream f("/tmp/test.txt");
        std::ostringstream n;
        time_t last = time(NULL);
        int last_i = 0;
        for (int i = 0; i < 1000000; i++)
        {
            t.id = random();
            n.str("");
            n << "N_" << t.id;
            t.name = n.str();
            t.insert();
            f << t.id << std::endl;
            time_t now = time(NULL);
            if (now != last)
            {
                printf("sync %d %d\n", i - last_i, i);
                AES_VFS::sqlite3_vfs_aes::sync();
                last = now;
                last_i = i;
            }
        }
    } // destructor for "t" releases locks held.

    r = sqlite3_close(pdb);
    if (r != SQLITE_OK)
        printf("ERR! close returns %d\n", r);

    sqlite3_shutdown();

    return 0;
}
