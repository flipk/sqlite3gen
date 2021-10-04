
#define __STDC_FORMAT_MACROS

#ifndef DEPENDING
#include SAMPLE_H_HDR
#endif

#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <gtest/gtest.h>

#include "sample_test.h"

int
main(int argc, char ** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    unlink(TEST_DATABASE);
    return ret;
}



//////////////////////////// SQL_TABLE_user_custom ////////////////////////////


SQL_TABLE_user_custom :: SQL_TABLE_user_custom(sqlite3 *_pdb /*= NULL*/)
    : SQL_TABLE_User(_pdb)
{
    pStmt_by_great_balance = NULL;
}

SQL_TABLE_user_custom :: ~SQL_TABLE_user_custom(void)
{
    sqlite3_finalize(pStmt_by_great_balance);
}


// note this is intended to demonstrate extended queries
// even though it duplicates the get_great_balance CUSTOM-GET
// in the base class.
bool
SQL_TABLE_user_custom :: get_by_great_balance(double threshold)
{
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

    if (library::SQL_TABLE_ALL_TABLES::log_get_func)
        library::SQL_TABLE_ALL_TABLES::log_get_func(
            library::SQL_TABLE_ALL_TABLES::log_arg,
            pStmt_by_great_balance);

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

void
SQL_TABLE_user_custom :: print(void)
{
    // we can print a blob as a string only because
    // this test program always sets 'blob' objects to
    // strings.
    printf("row %" PRId64 " userid %d"
           " %s %s %s SSN %d %f proto (%d) '%s'\n",
           (int64_t) rowid, userid,
           firstname.c_str(), mi.c_str(), lastname.c_str(),
           SSN, balance, (int) proto.length(), proto.c_str());
    printf("%d checkouts:\n", (int) Checkouts.size());
    for (size_t ind = 0; ind < Checkouts.size(); ind++)
    {
        printf("   row %" PRId64 " userid2 %d"
               " bookid %d duedate %" PRId64 "\n",
               (int64_t) Checkouts[ind].rowid,   Checkouts[ind].userid2,
               Checkouts[ind].bookid2, Checkouts[ind].duedate);
    }
    printf("TOSTRING: %s\n", toString().c_str());
}

////////////////////////////// SampleTextFixture //////////////////////////////

int
SampleTextFixture :: get_all(void)
{
    int count = 0;
    ucust.init();
    if (ucust.get_all() == false)
    {
        printf("get failed\n");
        return -1;
    }
    do {
        ucust.print();
        count++;
    } while (ucust.get_next());
    return count;
}

void
SampleTextFixture :: get_row(int64_t row)
{
    ucust.init();
    if (ucust.get_by_rowid(row) == false)
    {
        printf("get failed\n");
        return;
    }
    do {
        ucust.print();
    } while (ucust.get_next());
}

int
SampleTextFixture :: get_like(const std::string &patt)
{
    int count = 0;
    ucust.init();
    if (ucust.get_by_lastname_like(patt) == false)
    {
        printf("get like failed\n");
        return -1;
    }
    do {
        ucust.print();
        count++;
    } while (ucust.get_next());
    return count;
}

int
SampleTextFixture :: get_custom1(double thresh)
{
    int count = 0;
    ucust.init();
    if (ucust.get_great_balance(thresh) == false)
    {
        printf("get by great threshold returned no users\n");
        return -1;
    }
    do {
        ucust.print();
        count++;
    } while (ucust.get_next());
    return count;
}

int
SampleTextFixture :: get_custom2(const std::string &first,
                                 const std::string &last)
{
    int count = 0;
    ucust.init();
    if (ucust.get_firstlast(first, last) == false)
    {
        printf("get by firstlast returned no users\n");
        return -1;
    }
    do {
        ucust.print();
        count++;
    } while (ucust.get_next());
    return count;
}

void
SampleTextFixture :: log_sql_upd(void *arg, sqlite3_stmt *stmt)
{
    SampleTextFixture * stf = (SampleTextFixture*) arg;
    char * sql = sqlite3_expanded_sql(stmt);
    printf("** SQL UPDATE LOG: %s\n", sql);
    sqlite3_free(sql);
}

// static
void
SampleTextFixture :: log_sql_get(void *arg, sqlite3_stmt *stmt)
{
    SampleTextFixture * stf = (SampleTextFixture*) arg;
    char * sql = sqlite3_expanded_sql(stmt);
    printf("** SQL GET LOG: %s\n", sql);
    sqlite3_free(sql);
}

// static
void
SampleTextFixture :: log_sql_row(void *arg, const std::string &msg)
{
    SampleTextFixture * stf = (SampleTextFixture*) arg;
    fprintf(stderr, "** SQL ROW: %s\n", msg.c_str());
}

// static
void
SampleTextFixture :: log_sql_err(void *arg, const std::string &msg)
{
    SampleTextFixture * stf = (SampleTextFixture*) arg;
    fprintf(stderr, "** SQL ERROR: %s\n", msg.c_str());
}

// static
void
SampleTextFixture :: table_callback(sqlite3 *pdb, const std::string &table_name,
                                    int before, int after)
{
    printf("table '%s' goes from version %d to %d\n",
           table_name.c_str(), before, after);
}

void
SampleTextFixture :: SetUp()
{
    int sqlret = sqlite3_open(TEST_DATABASE, &pdb);
    ASSERT_EQ(sqlret,SQLITE_OK);
    user.set_db(pdb);
    ucust.set_db(pdb);
    library::SQL_TABLE_ALL_TABLES::register_log_funcs(
        &log_sql_upd, &log_sql_get, &log_sql_row, &log_sql_err,
        (void*) this);
    bool init_ok =
        library::SQL_TABLE_ALL_TABLES::init_all(pdb, &table_callback);
    ASSERT_EQ(init_ok,true);
}

void
SampleTextFixture :: TearDown()
{
    // this finalizes any open stmts and releases
    // reference counts/locks.
    user.set_db(NULL);
    ucust.set_db(NULL);
    ASSERT_EQ(ucust.was_properly_done(),true);

    int r = sqlite3_close(pdb);
    ASSERT_EQ(r,SQLITE_OK);
    sqlite3_shutdown();
}

///////////////////////////////// test cases /////////////////////////////////

TEST_F(SampleTextFixture, 1_descriptors)
{
    std::vector<library::SQL_Column_Descriptor>  cols;
    library::SQL_TABLE_User::get_column_descriptors(cols);
    printf("USER table column descriptors:\n");
    for (size_t ind = 0; ind < cols.size(); ind++)
    {
        library::SQL_Column_Descriptor &c = cols[ind];
        printf("tab '%s' field '%s' ctype '%s' sqty %d genty %d\n",
               c.tablename.c_str(), c.fieldname.c_str(),
               c.ctype.c_str(), c.sqlite_type, c.sqlite3gen_type);
    }
}

static int32_t userid_2to3 = -1;

TEST_F(SampleTextFixture, 2_insert)
{
    user.firstname = "flippy";
    user.lastname = "kadoodle";
    user.mi = "f";
    user.SSN = 456789012;
    user.balance = 14.92;
    user.proto = "PROTOBUFS BABY";
    ASSERT_EQ(user.insert(),true);

    userid_2to3 = user.userid;

    printf("inserted rowid %" PRId64 " userid %d\n",
           (int64_t) user.rowid, user.userid);

    ASSERT_EQ(get_all(),1);
}

TEST_F(SampleTextFixture, 3_getuserid)
{
    if (userid_2to3 == -1)
    {
        printf("ERROR : test 2 must be run before test 3....\n");
        ASSERT_NE(userid_2to3,-1);
    }

    ASSERT_EQ(ucust.get_by_userid(userid_2to3),true);
    ucust.print();

    ASSERT_EQ(ucust.lastname,"kadoodle");
    ASSERT_EQ(ucust.SSN,456789012);
}

TEST_F(SampleTextFixture, 4_getSSN)
{
    ASSERT_EQ(user.get_by_SSN(456789012),true);
    user.balance = 15.44;
    ASSERT_EQ(user.update_balance(),true);
    printf("updated balance in row %" PRId64 "\n", (int64_t) user.rowid);
}

#ifdef INCLUDE_SQLITE3GEN_PROTOBUF_SUPPORT

TEST_F(SampleTextFixture, 5_protobuf)
{
    ASSERT_EQ(user.get_by_SSN(456789012),true);

    printf("encoding to protobuf!\n");

    library::TABLE_User_m  msg;
    std::string binary;
    user.copy_to_proto(msg);
    msg.SerializeToString(&binary);
    printf("encoded user to protobuf %d bytes long\n",
           (int) binary.size());

    ASSERT_EQ((int) binary.size(),60);

    printf("decoding back from protobuf!\n");

    msg.Clear();
    ASSERT_EQ(msg.ParseFromString(binary),true);

    SQL_TABLE_user_custom t;
    t.copy_from_proto(msg);
    t.print();

    ASSERT_EQ(t.SSN,456789012);
    ASSERT_EQ(t.lastname,"kadoodle");

#endif

}

TEST_F(SampleTextFixture,6_like)
{
    ASSERT_EQ(get_like("%dood%"),1);
}

TEST_F(SampleTextFixture,7_custom1)
{
    ASSERT_EQ(get_custom1(15.00),1);
}

TEST_F(SampleTextFixture,8_custom2)
{
    ASSERT_EQ(get_custom2("flip%", "kad%"),1);
}

static int64_t rowid_9to10 = -1;

TEST_F(SampleTextFixture,9_delete_SSN)
{
    ASSERT_EQ(user.delete_SSN(456789012),true);

    // put it back again

    user.firstname = "flippy";
    user.lastname = "kadoodle";
    user.mi = "f";
    user.SSN = 456789012;
    user.balance = 14.92;
    user.proto = "PROTOBUFS BABY";
    ASSERT_EQ(user.insert(),true);

    rowid_9to10 = user.rowid;
}

TEST_F(SampleTextFixture,10_delete_rowid)
{
    if (rowid_9to10 == -1)
    {
        printf("ERROR test 9 must be run before test 10\n");
        ASSERT_NE(rowid_9to10,-1);
    }

    user.rowid = rowid_9to10;
    ASSERT_EQ(user.delete_rowid(),true);
}

static int32_t u1, u2, u3, b1, b2, b3;
static bool test11run = false;;

TEST_F(SampleTextFixture,11_insert_books)
{
    library::SQL_TRANSACTION t(pdb);

    ASSERT_EQ(t.begin(),true);

    {
        library::SQL_TABLE_User      u(pdb);
        library::SQL_TABLE_Book      b(pdb);
        library::SQL_TABLE_Checkouts c(pdb);

        u.firstname = "fir1";
        u.lastname = "las1";
        u.SSN = 11;
        u.balance = 4.25;
        u.proto = "this is proto blob";
#ifdef INCLUDE_SQLITE3GEN_PROTOBUF_SUPPORT
        u.test3 = sample::library2::ENUM_TWO;
#endif
        ASSERT_EQ(u.insert(),true);
        u1 = u.userid;

        u.firstname = "fir2";
        u.lastname = "las2";
        u.SSN = 22;
        u.balance = 8.50;
        u.proto = "this is proto blob 2";
#ifdef INCLUDE_SQLITE3GEN_PROTOBUF_SUPPORT
        u.test3 = sample::library2::ENUM_ONE;
#endif
        ASSERT_EQ(u.insert(),true);
        u2 = u.userid;

        u.firstname = "fir3";
        u.lastname = "las3";
        u.SSN = 33;
        u.balance = 12.75;
        u.proto = "this is proto blob 3";
#ifdef INCLUDE_SQLITE3GEN_PROTOBUF_SUPPORT
        u.test3 = sample::library2::ENUM_TWO;
#endif
        ASSERT_EQ(u.insert(),true);
        u3 = u.userid;

        printf("inserted userids %d, %d, and %d\n", u1, u2, u3);

        b.title = "book 1 title";
        b.isbn = "111222";
        b.price = 4.55;
        b.quantity = 6;
        ASSERT_EQ(b.insert(),true);
        b1 = b.bookid;

        b.title = "book 2 title";
        b.isbn = "444555";
        b.price = 8.15;
        b.quantity = 1;
        ASSERT_EQ(b.insert(),true);
        b2 = b.bookid;

        b.title = "book 3 title";
        b.isbn = "12345";
        b.price = 12.35;
        b.quantity = 2;
        ASSERT_EQ(b.insert(),true);
        b3 = b.bookid;

        printf("inserted bookids %d, %d, and %d\n", b1, b2, b3);

        c.bookid2 = b2;
        c.userid2 = u1;
        c.duedate = 5;
        ASSERT_EQ(c.insert(),true);

        c.bookid2 = b3;
        c.userid2 = u1;
        c.duedate = 6;
        ASSERT_EQ(c.insert(),true);

        c.bookid2 = b1;
        c.userid2 = u2;
        c.duedate = 12;
        ASSERT_EQ(c.insert(),true);

        c.bookid2 = b3;
        c.userid2 = u3;
        c.duedate = 2;
        ASSERT_EQ(c.insert(),true);

        printf("inserted 4 checkouts\n");
    }

    ASSERT_EQ(t.commit(),true);
    test11run = true;
}

TEST_F(SampleTextFixture,12_checkouts_protobuf)
{
    if (test11run == false)
    {
        printf("ERROR: 11 must be run before 12\n");
        ASSERT_EQ(test11run,true);
    }

#ifdef INCLUDE_SQLITE3GEN_PROTOBUF_SUPPORT

    SQL_TABLE_user_custom   u2(pdb);

    ASSERT_EQ(ucust.get_by_userid(u1),true);

    printf("got userid %d!\n", u1);
    ASSERT_GT(ucust.get_subtable_Checkouts(),0);
    ucust.print();

    library::TABLE_User_m  msg;
    ucust.copy_to_proto(msg);

    u2.copy_from_proto(msg);
    printf("after protobuf marshaling:\n");
    u2.print();

    ASSERT_EQ(u2.firstname,"fir1");
    ASSERT_EQ(u2.lastname,"las1");
    ASSERT_EQ(u2.SSN,11);
    ASSERT_EQ(u2.Checkouts.size(),2);

#endif

}

TEST_F(SampleTextFixture,13_checkouts_xml)
{
    if (test11run == false)
    {
        printf("ERROR: 11 must be run before 12\n");
        ASSERT_EQ(test11run,true);
    }

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

            ASSERT_EQ(doc.Parse( printer.CStr() ),tinyxml2::XML_SUCCESS);
            if (0) // i trust this to work
            {
                tinyxml2::XMLPrinter printer2;
                doc.Print( &printer2 );
                printf("reparsed xml:\n%s\n", printer2.CStr());
            }

            sqlite3 * pdb2;
            printf("CREATING SECOND TEST DATABASE\n");
            ASSERT_EQ(sqlite3_open(TEST_DATABASE2, &pdb2),SQLITE_OK);
            library::SQL_TABLE_ALL_TABLES::init_all(pdb2,
                                                    &SampleTextFixture::table_callback);
            library::SQL_TABLE_ALL_TABLES::import_xml_all(pdb2,doc);

            SQL_TABLE_user_custom u2(pdb2);

            printf(" *** displaying full contents of second test database\n");

            int count = 0;
            bool ok = u2.get_all();
            while (ok)
            {
                u2.get_subtables();
                u2.print();
                count++;
                ok = u2.get_next();
            }

            ASSERT_EQ(count,3);
            u2.set_db(NULL);
            ASSERT_EQ(sqlite3_close(pdb2),SQLITE_OK);
            unlink(TEST_DATABASE2);

            printf(" *** second test database done and gone\n");
        }
    }
#endif

}

TEST_F(SampleTextFixture,14_merge_from_proto)
{
    if (ucust.get_by_userid(u2))
    {
        printf("BEFORE MERGE:\n");
        ucust.print();

        library::TABLE_User_m msg;
        msg.set_lastname("las2");
        ASSERT_EQ(ucust.merge_from_proto(msg),false);
        msg.set_mi("x");
        ASSERT_EQ(ucust.merge_from_proto(msg),true);

        printf("AFTER MERGE:\n");
        ucust.print();
    }
}

TEST_F(SampleTextFixture,15_due1)
{
    library::SQL_SELECT_due_books  due(pdb);

    printf(" *** testing SQL_SELECT_due_books:\n");

    bool ok = due.get(1,4);
    ASSERT_EQ(ok,true);
    int count = 0;
    while (ok)
    {
        printf("user_rid %" PRId64 " %s %s "
               "book_rid %" PRId64 " %s "
               "checkouts_rid %" PRId64 " due %" PRId64 "\n",
               (int64_t) due.User_rowid,
               due.User_firstname.c_str(),
               due.User_lastname.c_str(),
               (int64_t) due.Book_rowid,
               due.Book_title.c_str(),
               (int64_t) due.Checkouts_rowid,
               (int64_t) due.Checkouts_duedate);
        printf("TOSTRING: %s\n", due.toString().c_str());
        count++;
        ok = due.get_next();
    }
    ASSERT_EQ(count,3);
}

TEST_F(SampleTextFixture,16_due2)
{
    library::SQL_SELECT_due_books2  due(pdb);

    printf(" *** testing SQL_SELECT_due_books2:\n");

    bool ok = due.get(1,4);
    ASSERT_EQ(ok,true);
    int count = 0;
    while (ok)
    {
        printf("user_rid %" PRId64 " %s %s "
               "book_rid %" PRId64 " %s "
               "checkouts_rid %" PRId64 " due %" PRId64 "\n",
               (int64_t) due.User_rowid,
               due.User_firstname.c_str(),
               due.User_lastname.c_str(),
               (int64_t) due.Book_rowid,
               due.Book_title.c_str(),
               (int64_t) due.Checkouts_rowid,
               (int64_t) due.Checkouts_duedate);
        printf("TOSTRING: %s\n", due.toString().c_str());
        count++;
        ok = due.get_next();
    }
    ASSERT_EQ(count,3);
}
