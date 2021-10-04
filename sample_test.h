
#define TEST_DATABASE  "build_native/sample_test.db"
#define TEST_DATABASE2 "build_native/sample_test2.db"

class SQL_TABLE_user_custom : public library::SQL_TABLE_User {
    sqlite3_stmt * pStmt_by_great_balance;
public:
    SQL_TABLE_user_custom(sqlite3 *_pdb = NULL);
    ~SQL_TABLE_user_custom(void);
    // note this is intended to demonstrate extended queries
    // even though it duplicates the get_great_balance CUSTOM-GET
    // in the base class.
    bool get_by_great_balance(double threshold);
    void print(void);
    bool was_properly_done(void) const { return previous_get == NULL; }
};

class SampleTextFixture : public ::testing::Test
{
private:
    static void log_sql_upd(void *arg, sqlite3_stmt *stmt);
    static void log_sql_get(void *arg, sqlite3_stmt *stmt);
    static void log_sql_row(void *arg, const std::string &msg);
    static void log_sql_err(void *arg, const std::string &msg);

protected:
    SQL_TABLE_user_custom ucust;
    library::SQL_TABLE_User  user;
    sqlite3 * pdb;

    void get_row(int64_t row);
    int get_all(void);
    // the following return how many rows they got.
    int get_like(const std::string &patt);
    int get_custom1(double thresh);
    int get_custom2(const std::string &first,
                     const std::string &last);
    static void table_callback(sqlite3 *pdb, const std::string &table_name,
                               int before, int after);

    void SetUp();
    void TearDown();
};

