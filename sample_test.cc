
#define __STDC_FORMAT_MACROS

#ifndef DEPENDING
#include SAMPLE_H_HDR
#endif

#include <stdio.h>
#include <inttypes.h>

class SQL_TABLE_user_custom : public library::SQL_TABLE_user {
    sqlite3_stmt * pStmt_by_great_balance;
public:
    SQL_TABLE_user_custom(sqlite3 *_pdb = NULL)
        : SQL_TABLE_user(_pdb)
    {
        pStmt_by_great_balance = NULL;
    }
    ~SQL_TABLE_user_custom(void)
    {
        sqlite3_finalize(pStmt_by_great_balance);
    }
    // note this is intended to demonstrate extended queries
    // even though it duplicates the get_great_balance CUSTOM-GET
    // in the base class.
    bool get_by_great_balance(double threshold) {
        int r;
        bool ret = false;

        if (pStmt_by_great_balance == NULL)
        {
            r = sqlite3_prepare_v2(
                pdb,
                "SELECT rowid,* FROM user WHERE balance > ?",
                -1, &pStmt_by_great_balance, NULL);
            if (r != SQLITE_OK)
                printf("ERROR building SELECT for balance at line %d\n",
                       __LINE__);
        }

        sqlite3_reset(pStmt_by_great_balance);

        r = sqlite3_bind_double(pStmt_by_great_balance, 1, threshold);
        if (r != SQLITE_OK)
        {
            fprintf(stderr, "SQL_TABLE_user :: get_by_great_balance : "
                    "bind: r = %d\n", r);
            return false;
        }

        if (log_get_func)
            log_get_func(log_arg, pStmt_by_great_balance);

        r = sqlite3_step(pStmt_by_great_balance);
        if (r == SQLITE_ROW)
        {
            ret = get_columns(pStmt_by_great_balance);
            previous_get = pStmt_by_great_balance;
        }
        else if (r == SQLITE_DONE)
            previous_get = NULL;

        return ret;
    }
    void print(void) {
        printf("row %" PRId64 " userid %" PRId64
               " %s %s %s SSN %d %f proto (%d)\n",
               (int64_t) rowid, userid,
               firstname.c_str(), mi.c_str(), lastname.c_str(),
               SSN, balance, (int) proto.length());
    }
};

SQL_TABLE_user_custom u;

void
get_all(sqlite3 *pdb)
{
    u.init();
    if (u.get_all() == false)
    {
        printf("get failed\n");
        return;
    }
    do {
        u.print();
    } while (u.get_next());
}

void
get_row(sqlite3 *pdb, int64_t row)
{
    u.init();
    if (u.get_by_rowid(row) == false)
    {
        printf("get failed\n");
        return;
    }
    do {
        u.print();
    } while (u.get_next());
}

void
get_like(sqlite3 *pdb, const std::string &patt)
{
    u.init();
    if (u.get_by_lastname_like(patt) == false)
    {
        printf("get like failed\n");
        return;
    }
    do {
        u.print();
    } while (u.get_next());
}

void
get_custom1(sqlite3 *pdb, double thresh)
{
    u.init();
    if (u.get_great_balance(thresh) == false)
    {
        printf("get by great threshold returned no users\n");
        return;
    }
    do {
        u.print();
    } while (u.get_next());
}

void
get_custom2(sqlite3 *pdb,
            const std::string &first,
            const std::string &last)
{
    u.init();
    if (u.get_firstlast(first, last) == false)
    {
        printf("get by firstlast returned no users\n");
        return;
    }
    do {
        u.print();
    } while (u.get_next());
}

void
test_protobuf(sqlite3 *pdb, int64_t userid)
{
    u.init();
    if (u.get_by_userid(userid) == false)
    {
        printf("cannot get by userid\n");
        return;
    }
    SQL_TABLE_user_custom  t(pdb);
    do {
        library::TABLE_user_m  msg;
        std::string binary;
        u.CopyToProto(msg);
        msg.SerializeToString(&binary);
        printf("encoded user to protobuf %d bytes long\n",
               (int) binary.size());
        msg.Clear();
        msg.ParseFromString(binary);
        t.CopyFromProto(msg);
        t.print();
    } while (u.get_next());
}

void log_sql_upd(void *arg, sqlite3_stmt *stmt)
{
    char * sql = sqlite3_expanded_sql(stmt);
    printf("** SQL UPDATE LOG: %s\n", sql);
    sqlite3_free(sql);
}

void log_sql_get(void *arg, sqlite3_stmt *stmt)
{
    char * sql = sqlite3_expanded_sql(stmt);
    printf("** SQL GET LOG: %s\n", sql);
    sqlite3_free(sql);
}

void table_callback(sqlite3 *pdb, const std::string &table_name,
                    int before, int after)
{
    printf("table '%s' goes from version %d to %d\n",
           table_name.c_str(), before, after);
}

int
main()
{
    sqlite3 * pdb;
    library::SQL_TABLE_user  user;

    printf("UPDATE_BALANCE = '%s'\n",  getenv("UPDATE_BALANCE"));
    printf("DELETE_BY_ROWID = '%s'\n", getenv("DELETE_BY_ROWID"));
    printf("RETAIN_TABLES = '%s'\n",   getenv("RETAIN_TABLES"));

    sqlite3_open("build_native/sample_test.db", &pdb);
    user.set_db(pdb);
    u.set_db(pdb);

    library::SQL_TABLE_ALL_TABLES::init_all(pdb, &table_callback);
    library::SQL_TABLE_user::register_log_funcs(
        &log_sql_upd, &log_sql_get, NULL);
    {

        user.userid = 4;
        user.firstname = "flippy";
        user.lastname = "kadoodle";
        user.mi = "f";
        user.SSN = 456789012;
        user.balance = 14.92;
        user.proto = "PROTOBUFS BABY";

        user.insert();
        printf("inserted row %" PRId64 "\n", (int64_t) user.rowid);

        get_all(pdb);

        user.balance = 15.44;

        if (getenv("UPDATE_BALANCE") == NULL)
            user.update(); // test both
        else
            user.update_balance();

        printf("updated row %" PRId64 "\n", (int64_t) user.rowid);

        test_protobuf(pdb, 4);

        get_row(pdb, user.rowid);

        get_all(pdb);

        get_like(pdb, "%dood%");

        get_custom1(pdb, 15.00);
        get_custom2(pdb, "flip%", "kad%");

        if (getenv("DELETE_BY_ROWID") == NULL)
            user.delete_SSN(456789012);
        else
            user.delete_rowid();
    }

    // if every query completes until SQLITE_DONE,
    // these aren't needed.
//    user.finalize();
//    u.finalize();
//    library::SQL_TABLE_user::table_drop(pdb);
    if (getenv("RETAIN_TABLES") == NULL)
        library::SQL_TABLE_ALL_TABLES::table_drop_all(pdb);
    sqlite3_close(pdb);
    return 0;
}
