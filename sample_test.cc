
#define __STDC_FORMAT_MACROS

#ifndef DEPENDING
#include SAMPLE_H_HDR
#endif

#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>

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
        // we can print a blob as a string only because
        // this test program always sets 'blob' objects to
        // strings.
        printf("row %" PRId64 " userid %d"
               " %s %s %s SSN %d %f proto (%d) '%s'\n",
               (int64_t) rowid, userid,
               firstname.c_str(), mi.c_str(), lastname.c_str(),
               SSN, balance, (int) proto.length(), proto.c_str());
        printf("%d checkouts:\n", (int) checkouts.size());
        for (size_t ind = 0; ind < checkouts.size(); ind++)
        {
            printf("   row %" PRId64 " userid2 %d"
                   " bookid %d duedate %" PRId64 "\n",
                   (int64_t) checkouts[ind].rowid,   checkouts[ind].userid2,
                   checkouts[ind].bookid2, checkouts[ind].duedate);
        }
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

#ifdef INCLUDE_SQLITE3GEN_PROTOBUF_SUPPORT
void
test_protobuf(sqlite3 *pdb, int32_t userid)
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
        u.copy_to_proto(msg);
        msg.SerializeToString(&binary);
        printf("encoded user to protobuf %d bytes long\n",
               (int) binary.size());
        msg.Clear();
        msg.ParseFromString(binary);
        t.copy_from_proto(msg);
        t.print();
    } while (u.get_next());
}
#endif

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

void log_sql_row(void *arg, const std::string &msg)
{
    fprintf(stderr, "** SQL ROW: %s\n", msg.c_str());
}

void log_sql_err(void *arg, const std::string &msg)
{
    fprintf(stderr, "** SQL ERROR: %s\n", msg.c_str());
}

void table_callback(sqlite3 *pdb, const std::string &table_name,
                    int before, int after)
{
    printf("table '%s' goes from version %d to %d\n",
           table_name.c_str(), before, after);
}

void test_subtables(sqlite3 * pdb);

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

    library::SQL_TABLE_ALL_TABLES::register_log_funcs(
        &log_sql_upd, &log_sql_get, &log_sql_row, &log_sql_err, NULL);
    if (!library::SQL_TABLE_ALL_TABLES::init_all(pdb, &table_callback))
        goto bail;

    {

        user.firstname = "flippy";
        user.lastname = "kadoodle";
        user.mi = "f";
        user.SSN = 456789012;
        user.balance = 14.92;
        user.proto = "PROTOBUFS BABY";

        user.insert();
        printf("inserted rowid %" PRId64 " userid %d\n",
               (int64_t) user.rowid, user.userid);

        get_all(pdb);

        user.balance = 15.44;

        if (getenv("UPDATE_BALANCE") == NULL)
            user.update(); // test both
        else
            user.update_balance();

        printf("updated row %" PRId64 "\n", (int64_t) user.rowid);

#ifdef INCLUDE_SQLITE3GEN_PROTOBUF_SUPPORT
        test_protobuf(pdb, 1);
#endif

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

    test_subtables(pdb);

bail:

    // if every query completes until SQLITE_DONE,
    // these aren't needed.
//    user.finalize();
//    u.finalize();
//    library::SQL_TABLE_user::table_drop(pdb);
    if (getenv("RETAIN_TABLES") == NULL)
        library::SQL_TABLE_ALL_TABLES::table_drop_all(pdb);

    // release user and u to release locks.
    user.set_db(NULL);
    u.set_db(NULL);

    int r = sqlite3_close(pdb);
    if (r != SQLITE_OK)
        printf("ERR!  close returns %d\n", r);
    sqlite3_shutdown();
    return 0;
}

void
test_subtables(sqlite3 * pdb)
{
    int32_t u1, u2, u3, b1, b2, b3;

    library::SQL_TRANSACTION t(pdb);

    if (t.begin())
        printf(" ** TRANSACTION BEGUN\n");
    else
    {
        printf(" ** TRANSACTION FAILED\n");
        exit(1);
    }

    {
        library::SQL_TABLE_user      u(pdb);
        library::SQL_TABLE_book      b(pdb);
        library::SQL_TABLE_checkouts c(pdb);

        u.firstname = "fir1";
        u.lastname = "las1";
        u.SSN = 11;
        u.balance = 4.25;
        u.proto = "this is proto blob";
#ifdef INCLUDE_SQLITE3GEN_PROTOBUF_SUPPORT
        u.test3 = sample::library2::ENUM_TWO;
#endif
        u.insert();
        u1 = u.userid;

        u.firstname = "fir2";
        u.lastname = "las2";
        u.SSN = 22;
        u.balance = 8.50;
        u.proto = "this is proto blob 2";
#ifdef INCLUDE_SQLITE3GEN_PROTOBUF_SUPPORT
        u.test3 = sample::library2::ENUM_ONE;
#endif
        u.insert();
        u2 = u.userid;

        u.firstname = "fir3";
        u.lastname = "las3";
        u.SSN = 33;
        u.balance = 12.75;
        u.proto = "this is proto blob 3";
#ifdef INCLUDE_SQLITE3GEN_PROTOBUF_SUPPORT
        u.test3 = sample::library2::ENUM_TWO;
#endif
        u.insert();
        u3 = u.userid;

        printf("inserted userids %d, %d, and %d\n", u1, u2, u3);

        b.title = "book 1 title";
        b.isbn = "111222";
        b.price = 4.55;
        b.quantity = 6;
        b.insert();
        b1 = b.bookid;

        b.title = "book 2 title";
        b.isbn = "444555";
        b.price = 8.15;
        b.quantity = 1;
        b.insert();
        b2 = b.bookid;

        b.title = "book 3 title";
        b.isbn = "12345";
        b.price = 12.35;
        b.quantity = 2;
        b.insert();
        b3 = b.bookid;

        printf("inserted bookids %d, %d, and %d\n", b1, b2, b3);

        c.bookid2 = b2;
        c.userid2 = u1;
        c.duedate = 5;
        c.insert();

        c.bookid2 = b3;
        c.userid2 = u1;
        c.duedate = 6;
        c.insert();

        c.bookid2 = b1;
        c.userid2 = u2;
        c.duedate = 12;
        c.insert();

        c.bookid2 = b3;
        c.userid2 = u3;
        c.duedate = 2;
        c.insert();

        printf("inserted 4 checkouts\n");
    }

    if (t.commit())
        printf(" ** TRANSACTION COMPLETE\n");
    else
    {
        printf(" ** TRANSACTION FAILED\n");
        exit(1);
    }

#ifdef INCLUDE_SQLITE3GEN_PROTOBUF_SUPPORT
    {
        SQL_TABLE_user_custom        u(pdb);

        if (u.get_by_userid(u1))
        {
            printf("got userid %d!\n", u1);
            u.get_subtable_checkouts();
            u.print();
            library::TABLE_user_m  msg;
            u.copy_to_proto(msg);

            {
                SQL_TABLE_user_custom  u2(pdb);
                u2.copy_from_proto(msg);
                printf("after protobuf marshaling:\n");
                u2.print();
            }
        }
    }
#endif

#ifdef INCLUDE_SQLITE3GEN_TINYXML2_SUPPORT
    {
        tinyxml2::XMLPrinter printer;

        {
            tinyxml2::XMLDocument   doc;
            library::SQL_TABLE_ALL_TABLES :: export_xml_all(pdb,doc);
            doc.Print( &printer );
            printf("xml:\n%s\n", printer.CStr());
        }

        {
            tinyxml2::XMLDocument  doc;

            doc.Parse( printer.CStr() );
            if (0) // i trust this to work
            {
                tinyxml2::XMLPrinter printer2;
                doc.Print( &printer2 );
                printf("reparsed xml:\n%s\n", printer2.CStr());
            }

            sqlite3 * pdb2;
            printf("CREATING SECOND TEST DATABASE\n");
            sqlite3_open("build_native/sample_test2.db", &pdb2);
            library::SQL_TABLE_ALL_TABLES::init_all(pdb2, &table_callback);
            library::SQL_TABLE_ALL_TABLES::import_xml_all(pdb2,doc);

            SQL_TABLE_user_custom        u(pdb2);

            printf(" *** displaying full contents of second test database\n");

            bool ok = u.get_all();
            while (ok)
            {
                u.get_subtables();
                u.print();
                ok = u.get_next();
            }

            u.set_db(NULL);

            int r = sqlite3_close(pdb2);
            if (r != SQLITE_OK)
                printf("ERR!  close returns %d\n", r);
            unlink("build_native/sample_test2.db");

            printf(" *** second test database done and gone\n");
        }
    }
#endif

    {
        library::SQL_SELECT_due_books  due(pdb);

        printf(" *** testing SQL_SELECT_due_books:\n");

        bool ok = due.get(1,4);
        while (ok)
        {
            printf("user_rid %" PRId64 " %s %s "
                   "book_rid %" PRId64 " %s "
                   "checkouts_rid %" PRId64 " due %" PRId64 "\n",
                   (int64_t) due.user_rowid,
                   due.user_firstname.c_str(),
                   due.user_lastname.c_str(),
                   (int64_t) due.book_rowid,
                   due.book_title.c_str(),
                   (int64_t) due.checkouts_rowid,
                   (int64_t) due.checkouts_duedate);
            ok = due.get_next();
        }
    }

    {
        library::SQL_SELECT_due_books2  due(pdb);

        printf(" *** testing SQL_SELECT_due_books2:\n");

        bool ok = due.get(1,4);
        while (ok)
        {
            printf("user_rid %" PRId64 " %s %s "
                   "book_rid %" PRId64 " %s "
                   "checkouts_rid %" PRId64 " due %" PRId64 "\n",
                   (int64_t) due.user_rowid,
                   due.user_firstname.c_str(),
                   due.user_lastname.c_str(),
                   (int64_t) due.book_rowid,
                   due.book_title.c_str(),
                   (int64_t) due.checkouts_rowid,
                   (int64_t) due.checkouts_duedate);
            ok = due.get_next();
        }
    }

}
