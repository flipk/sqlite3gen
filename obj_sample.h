
///////////////////////////////////////////////////////////
// NOTE // NOTE // NOTE // NOTE // NOTE // NOTE // NOTE  //
//                                                       //
//       THIS FILE IS AUTOGENERATED BY sqlite3gen        //
// DO NOT EDIT THIS FILE, EDIT THE SOURCE AND REGENERATE //
//                                                       //
// NOTE // NOTE // NOTE // NOTE // NOTE // NOTE // NOTE  //
///////////////////////////////////////////////////////////

#include <inttypes.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include "sqlite3.h"
#include "tinyxml2.h"
#include "sample.pb.h"



/* header top line 1 */
/* header top line 2 */


namespace library {

typedef void (*sql_log_function_t)(void *arg, sqlite3_stmt *);
typedef void (*sql_err_function_t)(void *arg, const std::string &msg);
typedef void (*table_version_callback)(
    sqlite3 *pdb,
    const std::string &table_name,
    int version_in_file,
    int version_in_code);

class SQL_TABLE_user; // forward
class SQL_TABLE_book; // forward
class SQL_TABLE_checkouts; // forward


class SQL_TABLE_user {
    sqlite3_stmt * pStmt_insert;
    sqlite3_stmt * pStmt_update;
    sqlite3_stmt * pStmt_delete_rowid;
    sqlite3_stmt * pStmt_get_by_rowid;
    sqlite3_stmt * pStmt_get_all;
    sqlite3_stmt * pStmt_by_userid;
    sqlite3_stmt * pStmt_by_SSN;
    sqlite3_stmt * pStmt_by_test2;
    sqlite3_stmt * pStmt_by_test3;

    sqlite3_stmt * pStmt_by_lastname_like;

    sqlite3_stmt * pStmt_get_great_balance;
    sqlite3_stmt * pStmt_get_founders;
    sqlite3_stmt * pStmt_get_firstlast;

    sqlite3_stmt * pStmt_update_balance;
    sqlite3_stmt * pStmt_update_firstlast;
    sqlite3_stmt * pStmt_update_by_userid_stuff;

    sqlite3_stmt * pStmt_del_SSN;


protected:
    sqlite3 *pdb;
    sqlite3_stmt * previous_get;
    bool get_columns(sqlite3_stmt *pStmt);
    static sql_log_function_t log_upd_func;
    static sql_log_function_t log_get_func;
    static void * log_arg;
    static sql_err_function_t err_log_func;
    static void * err_log_arg;
    static void print_err(const char *func, int lineno,
                          const char *format, ...);

    typedef bool (SQL_TABLE_user::*xml_decoder_func_t)
        (const tinyxml2::XMLElement *el);
    typedef std::map<std::string,xml_decoder_func_t> xml_decoder_map_t;
    xml_decoder_map_t xml_decoders;
    bool xml_decoders_initialized;

    bool xml_decoder_userid(const tinyxml2::XMLElement *el);
    bool xml_decoder_firstname(const tinyxml2::XMLElement *el);
    bool xml_decoder_lastname(const tinyxml2::XMLElement *el);
    bool xml_decoder_mi(const tinyxml2::XMLElement *el);
    bool xml_decoder_SSN(const tinyxml2::XMLElement *el);
    bool xml_decoder_balance(const tinyxml2::XMLElement *el);
    bool xml_decoder_proto(const tinyxml2::XMLElement *el);
    bool xml_decoder_test2(const tinyxml2::XMLElement *el);
    bool xml_decoder_test3(const tinyxml2::XMLElement *el);
    bool xml_decoder_checkouts(const tinyxml2::XMLElement *el);


public:
    SQL_TABLE_user(sqlite3 *_pdb = NULL);
    SQL_TABLE_user(const SQL_TABLE_user &other);
    virtual ~SQL_TABLE_user(void);

    static const int TABLE_VERSION = 19;

    void init(void);
    void init_statements(void);
    void finalize(void);

    void set_db(sqlite3 *_pdb) {
        finalize();
        pdb = _pdb;
    }

    sqlite3_int64 rowid;

    int64_t userid;
    std::string firstname;
    std::string lastname;
    std::string mi;
    int32_t SSN;
    double balance;
    std::string proto;
    bool test2;
    sample::library2::EnumField_t test3;
    // NOTE this is only populated by get_subtable_checkouts()
    std::vector<SQL_TABLE_checkouts> checkouts;

    bool get_by_userid(int64_t v);
    bool get_by_SSN(int32_t v);
    bool get_by_test2(bool v);
    bool get_by_test3(sample::library2::EnumField_t v);
// note this assumes foreign key userid2 is populated;
// returns number of rows fetched.
    int get_subtable_checkouts(void);
// true if ok, false if failure inserting (duplicate keys?)
    bool insert_subtable_checkouts(void);

    // get all subtables
    void get_subtables(void);
    void insert_subtables(void);
    bool get_by_lastname_like(const std::string &patt);

// WHERE balance > ?
    bool get_great_balance(double v1);
// WHERE userid < 100
    bool get_founders(void);
// WHERE firstname LIKE ? AND lastname LIKE ?
    bool get_firstlast(const std::string & v1, const std::string & v2);

    bool get_next(void);
    bool insert(void); // updates rowid
    bool update(void);
    bool delete_rowid(void); // delete by rowid
    bool get_by_rowid(int64_t v1);
    bool get_all(void);
    bool update_balance(void);
    bool update_firstlast(void);
// WHERE userid = ? and lastname = ?
    bool update_by_userid_stuff(int64_t v1, const std::string & v2);

// WHERE ssn = ?
    bool delete_SSN(int32_t v1);

// NOTE these only copy SUBTABLEs if you have called the
//      get_subtable_* methods to populate them.
    void CopyToProto(library::TABLE_user_m &msg);
    void CopyFromProto(const library::TABLE_user_m &msg);

    void CopyToXml(tinyxml2::XMLElement *el);
    bool CopyFromXml(const tinyxml2::XMLElement *el);

    static void register_log_funcs(sql_log_function_t _upd_func,
                                   sql_log_function_t _get_func,
                                   void *_arg,
                                   sql_err_function_t _err_func,
                                   void *_err_arg)
    {
        log_upd_func = _upd_func;
        log_get_func = _get_func;
        log_arg  = _arg;
        err_log_func = _err_func;
        err_log_arg = _err_arg;
    }
    static bool init(sqlite3 *pdb, table_version_callback cb);
    static bool table_create(sqlite3 *pdb);
    static void table_drop(sqlite3 *pdb);
    static void export_xml(sqlite3 *pdb, tinyxml2::XMLElement *el);
    static bool import_xml(sqlite3 *pdb, tinyxml2::XMLElement *el);
};


class SQL_TABLE_book {
    sqlite3_stmt * pStmt_insert;
    sqlite3_stmt * pStmt_update;
    sqlite3_stmt * pStmt_delete_rowid;
    sqlite3_stmt * pStmt_get_by_rowid;
    sqlite3_stmt * pStmt_get_all;
    sqlite3_stmt * pStmt_by_bookid;
    sqlite3_stmt * pStmt_by_isbn;

    sqlite3_stmt * pStmt_by_title_like;

    sqlite3_stmt * pStmt_get_out_of_stock;

    sqlite3_stmt * pStmt_update_quantity;
    sqlite3_stmt * pStmt_update_price;



protected:
    sqlite3 *pdb;
    sqlite3_stmt * previous_get;
    bool get_columns(sqlite3_stmt *pStmt);
    static sql_log_function_t log_upd_func;
    static sql_log_function_t log_get_func;
    static void * log_arg;
    static sql_err_function_t err_log_func;
    static void * err_log_arg;
    static void print_err(const char *func, int lineno,
                          const char *format, ...);

    typedef bool (SQL_TABLE_book::*xml_decoder_func_t)
        (const tinyxml2::XMLElement *el);
    typedef std::map<std::string,xml_decoder_func_t> xml_decoder_map_t;
    xml_decoder_map_t xml_decoders;
    bool xml_decoders_initialized;

    bool xml_decoder_bookid(const tinyxml2::XMLElement *el);
    bool xml_decoder_title(const tinyxml2::XMLElement *el);
    bool xml_decoder_isbn(const tinyxml2::XMLElement *el);
    bool xml_decoder_price(const tinyxml2::XMLElement *el);
    bool xml_decoder_quantity(const tinyxml2::XMLElement *el);


public:
    SQL_TABLE_book(sqlite3 *_pdb = NULL);
    SQL_TABLE_book(const SQL_TABLE_book &other);
    virtual ~SQL_TABLE_book(void);

    static const int TABLE_VERSION = 1;

    void init(void);
    void init_statements(void);
    void finalize(void);

    void set_db(sqlite3 *_pdb) {
        finalize();
        pdb = _pdb;
    }

    sqlite3_int64 rowid;

    int64_t bookid;
    std::string title;
    std::string isbn;
    double price;
    int32_t quantity;

    bool get_by_bookid(int64_t v);
    bool get_by_isbn(const std::string & v);

    // get all subtables
    void get_subtables(void);
    void insert_subtables(void);
    bool get_by_title_like(const std::string &patt);

// WHERE quantity == 0
    bool get_out_of_stock(void);

    bool get_next(void);
    bool insert(void); // updates rowid
    bool update(void);
    bool delete_rowid(void); // delete by rowid
    bool get_by_rowid(int64_t v1);
    bool get_all(void);
    bool update_quantity(void);
    bool update_price(void);


// NOTE these only copy SUBTABLEs if you have called the
//      get_subtable_* methods to populate them.
    void CopyToProto(library::TABLE_book_m &msg);
    void CopyFromProto(const library::TABLE_book_m &msg);

    void CopyToXml(tinyxml2::XMLElement *el);
    bool CopyFromXml(const tinyxml2::XMLElement *el);

    static void register_log_funcs(sql_log_function_t _upd_func,
                                   sql_log_function_t _get_func,
                                   void *_arg,
                                   sql_err_function_t _err_func,
                                   void *_err_arg)
    {
        log_upd_func = _upd_func;
        log_get_func = _get_func;
        log_arg  = _arg;
        err_log_func = _err_func;
        err_log_arg = _err_arg;
    }
    static bool init(sqlite3 *pdb, table_version_callback cb);
    static bool table_create(sqlite3 *pdb);
    static void table_drop(sqlite3 *pdb);
    static void export_xml(sqlite3 *pdb, tinyxml2::XMLElement *el);
    static bool import_xml(sqlite3 *pdb, tinyxml2::XMLElement *el);
};


class SQL_TABLE_checkouts {
    sqlite3_stmt * pStmt_insert;
    sqlite3_stmt * pStmt_update;
    sqlite3_stmt * pStmt_delete_rowid;
    sqlite3_stmt * pStmt_get_by_rowid;
    sqlite3_stmt * pStmt_get_all;
    sqlite3_stmt * pStmt_by_bookid2;
    sqlite3_stmt * pStmt_by_userid2;


    sqlite3_stmt * pStmt_get_due_now;




protected:
    sqlite3 *pdb;
    sqlite3_stmt * previous_get;
    bool get_columns(sqlite3_stmt *pStmt);
    static sql_log_function_t log_upd_func;
    static sql_log_function_t log_get_func;
    static void * log_arg;
    static sql_err_function_t err_log_func;
    static void * err_log_arg;
    static void print_err(const char *func, int lineno,
                          const char *format, ...);

    typedef bool (SQL_TABLE_checkouts::*xml_decoder_func_t)
        (const tinyxml2::XMLElement *el);
    typedef std::map<std::string,xml_decoder_func_t> xml_decoder_map_t;
    xml_decoder_map_t xml_decoders;
    bool xml_decoders_initialized;

    bool xml_decoder_bookid2(const tinyxml2::XMLElement *el);
    bool xml_decoder_userid2(const tinyxml2::XMLElement *el);
    bool xml_decoder_duedate(const tinyxml2::XMLElement *el);


public:
    SQL_TABLE_checkouts(sqlite3 *_pdb = NULL);
    SQL_TABLE_checkouts(const SQL_TABLE_checkouts &other);
    virtual ~SQL_TABLE_checkouts(void);

    static const int TABLE_VERSION = 1;

    void init(void);
    void init_statements(void);
    void finalize(void);

    void set_db(sqlite3 *_pdb) {
        finalize();
        pdb = _pdb;
    }

    sqlite3_int64 rowid;

    int64_t bookid2;
    int64_t userid2;
    int64_t duedate;

    bool get_by_bookid2(int64_t v);
    bool get_by_userid2(int64_t v);

    // get all subtables
    void get_subtables(void);
    void insert_subtables(void);

// WHERE duedate < ?
    bool get_due_now(int64_t v1);

    bool get_next(void);
    bool insert(void); // updates rowid
    bool update(void);
    bool delete_rowid(void); // delete by rowid
    bool get_by_rowid(int64_t v1);
    bool get_all(void);


// NOTE these only copy SUBTABLEs if you have called the
//      get_subtable_* methods to populate them.
    void CopyToProto(library::TABLE_checkouts_m &msg);
    void CopyFromProto(const library::TABLE_checkouts_m &msg);

    void CopyToXml(tinyxml2::XMLElement *el);
    bool CopyFromXml(const tinyxml2::XMLElement *el);

    static void register_log_funcs(sql_log_function_t _upd_func,
                                   sql_log_function_t _get_func,
                                   void *_arg,
                                   sql_err_function_t _err_func,
                                   void *_err_arg)
    {
        log_upd_func = _upd_func;
        log_get_func = _get_func;
        log_arg  = _arg;
        err_log_func = _err_func;
        err_log_arg = _err_arg;
    }
    static bool init(sqlite3 *pdb, table_version_callback cb);
    static bool table_create(sqlite3 *pdb);
    static void table_drop(sqlite3 *pdb);
    static void export_xml(sqlite3 *pdb, tinyxml2::XMLElement *el);
    static bool import_xml(sqlite3 *pdb, tinyxml2::XMLElement *el);
};


class SQL_TABLE_ALL_TABLES {
public:
    static bool init_all(sqlite3 *pdb, table_version_callback cb);
    static void table_drop_all(sqlite3 *pdb);
    static void register_log_funcs(sql_log_function_t _upd_func,
                                   sql_log_function_t _get_func,
                                   void *_arg,
                                   sql_err_function_t _err_func,
                                   void *_err_arg);
    static void export_xml_all(sqlite3 *pdb, tinyxml2::XMLDocument &doc);
    static bool import_xml_all(sqlite3 *pdb, tinyxml2::XMLDocument &doc);
};

}; // namespace library


/* header bottom line 1 */
/* header bottom line 2 */


