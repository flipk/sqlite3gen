
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
        // we can print a blob as a string only because
        // this test program always sets 'blob' objects to
        // strings.
        printf("row %" PRId64 " userid %" PRId64
               " %s %s %s SSN %d %f proto (%d) '%s'\n",
               (int64_t) rowid, userid,
               firstname.c_str(), mi.c_str(), lastname.c_str(),
               SSN, balance, (int) proto.length(), proto.c_str());
        printf("%d checkouts:\n", (int) checkouts.size());
        for (size_t ind = 0; ind < checkouts.size(); ind++)
        {
            printf("   row %" PRId64 " userid2 %" PRId64
                   " bookid %" PRId64 " duedate %" PRId64 "\n",
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

    library::SQL_TABLE_user::register_log_funcs(
        &log_sql_upd, &log_sql_get, NULL, &log_sql_err, NULL);
    library::SQL_TABLE_ALL_TABLES::init_all(pdb, &table_callback);
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

    test_subtables(pdb);

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
    {
        library::SQL_TABLE_user      u(pdb);
        library::SQL_TABLE_book      b(pdb);
        library::SQL_TABLE_checkouts c(pdb);

        u.userid = 1;
        u.firstname = "fir1";
        u.lastname = "las1";
        u.SSN = 11;
        u.balance = 4.00;
        u.proto = "this is proto blob";
        u.insert();

        b.bookid = 2;
        b.title = "book 2 title";
        b.isbn = 4.44;
        b.price = 8.00;
        b.quantity = 1;
        b.insert();

        b.bookid = 3;
        b.title = "book 3 title";
        b.isbn = "12345";
        b.price = 12.00;
        b.quantity = 2;
        b.insert();

        c.bookid2 = 2;
        c.userid2 = 1;
        c.duedate = 5;
        c.insert();

        c.bookid2 = 3;
        c.userid2 = 1;
        c.duedate = 6;
        c.insert();
    }

    SQL_TABLE_user_custom        u(pdb);

    if (u.get_by_userid(1))
    {
        printf("got user id 1!\n");
        u.get_subtable_checkouts();
        u.print();
        library::TABLE_user_m  msg;
        u.CopyToProto(msg);

        {
            SQL_TABLE_user_custom  u2(pdb);
            u2.CopyFromProto(msg);
            printf("after protobuf marshaling:\n");
            u2.print();
        }

        MyXmlNode n;
        u.CopyToXmlNode(n);
        std::string xml;
        if (n.generate(xml) == false)
            printf("ERROR during XML generate\n");
        else
            printf("generated xml:\n%s\n", xml.c_str());

        n.init();
        if (n.parse(xml) == 0)
            printf("ERROR during XML parsing\n");
        else
        {
            SQL_TABLE_user_custom  u2(pdb);
            if (u2.CopyFromXmlNode(n) == false)
                printf("ERROR in CopyFromXml\n");
            else
            {
                printf("after xml marshaling:\n");
                u2.print();
            }
        }
    }


    {
        std::string xml(
"<user> "
"  <userid type=\"pod\"> 1 </userid>"
"  <firstname type=\"text\">fir1</firstname>"
"  <lastname type=\"text\">las1</lastname>"
"  <mi type=\"text\"/>"
"  <SSN type=\"pod\"> 11 </SSN>"
"  <balance type=\"pod\"> 4 </balance>"
"  <proto type=\"blob\">746869732069732070726f746f20626c6f62</proto>"
"  <test2 type=\"bool\">false</test2>"
"  <test3 type=\"sample::library2::EnumField_t\">ENUM_TWO</test3>"
"  <checkouts index=\"0\" type=\"subtable\">"
"    <bookid2 type=\"pod\">2</bookid2>"
"    <userid2 type=\"pod\">1</userid2>"
"    <duedate type=\"pod\">5</duedate>"
"  </checkouts>"
"  <checkouts index=\"1\" type=\"subtable\">"
"    <bookid2 type=\"pod\">3</bookid2>"
"    <userid2 type=\"pod\">1</userid2>"
"    <duedate type=\"pod\">6</duedate>"
"  </checkouts>"
"</user>"
            );

        MyXmlNode n;
        if (n.parse(xml) == 0)
            printf("ERROR during 2nd XML parsing\n");
        else
        {
            SQL_TABLE_user_custom  u2(pdb);
            if (u2.CopyFromXmlNode(n) == false)
                printf("ERROR in 2nd CopyFromXml\n");
            else
            {
                printf("after xml 2nd marshaling:\n");
                u2.print();
            }
        } 
    }
}
