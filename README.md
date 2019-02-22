
# SQLITE3GEN #

This tool autogenerates C++ classes for fetching and storing rows
in an [SQLITE3](https://www.sqlite.org/index.html) database. I also
like other libraries for doing this (I particularly recommend
[sqlite3pp](https://github.com/iwongu/sqlite3pp)), but I had a requirement
for this project that it work on a very hold hardware platform
which did not support C++11 features (sqlite3pp uses closures and other
features found only in C++11 or newer).

## samples

Refer to the file `sample.schema` in this source, and reference
`obj_sample.h` and `obj_sample.cc` to see what this tool produces.
`sample_test.cc` shows what code can do with these classes.

## format of schema file

Schema files may contain comments, preceded by `#` symbols. The entire
remainder of a line beginning with this character is ignored.

```
# this is a comment.
```

Tables are demarked with curly braces. The format for a table is thus:

```
TABLE <table-name>
{
     <column-name>  TYPE  ATTRIBUTES
     <repeat the above for each column>

     CUSTOM-GET  <get-method-name>  (<list-of-types>)
                 "<where-clause-for-custom-get>"

     CUSTOM-UPD  <update-method-name>  (<list-of-column-names>)

     CUSTOM-DEL  <delete-method-name>  (<list-of-types>)
                 "<where-clause-for-custom-delete>"
}
```

### keywords

Keywords must be in upper case. Recognized keywords are:

```
table starts with : TABLE
types : INT INT64 TEXT DOUBLE BLOB
column attributes : INDEX LIKEQUERY QUERY
customs : CUSTOM-GET CUSTOM-UPD CUSTOM-DEL
```

The `table-name` will be used to complete a C++ class of the form
`SQL_TABLE_<table-name>`. The column-names declare variables within
the C++ class.

### data types

The TYPE declares the SQL database data type. SQL
data types map to C++ data types as such:

| SQL type | C++ type |
| -------- | -------- |
|  INT     | int32_t  |
|  INT64   | int64_t  |
|  DOUBLE  | double   |
|  TEXT    | std::string |
|  BLOB    | std::string |

### INDEX, QUERY, and LIKEQUERY attributes

Placing the `INDEX` keyword after a column TYPE provides the
creation of the SQL INDEX in the database. This implies you wish to
speed up queries on that column (see SQLITE3 documentation for more
details on indexes).

Placing the `QUERY` keyword after a column TYPE instructs the
tool to add a method to the C++ class for retrieving from the
database based on that field.

```C++
bool get_by_<column-name>(TYPE v);
```

The TYPE in the above statement will be the C++ type from the above
table which maps to the SQL type for the column.

NOTE this also populates "rowid" in the class so a successive
`update` will update the same row in the table.

Placing the `LIKEQUERY` keyword instructs the tool to add a method
for retrieving based on an SQL "LIKE" pattern (see SQLITE3 documentation
for more information on the LIKE keyword).  This will add a method
that looks like this:

```C++
bool get_by_<column-name>_like(std::string v);
```

Note the argument of `v` for a `_like` method will always
be `std::string`.

### insert, update, get_by_rowid, delete_rowid methods

The following methods are always added to every SQL_TABLE class:

```C++
bool insert(void); // updates rowid
bool update(void);
bool delete_rowid(void); // delete by rowid
bool get_by_rowid(int64_t rowid);
```

Use the `insert` method to add a new row to the table. Note the `rowid`
member of the class is ignored, don't populate it before calling `insert`.
`insert` will populate it on return to show what new rowid was added
to the table.

The `update` method assumes `rowid` contains the current row id, and
writes all data members of the object to that row of the table. Behavior
is undefined if you call `update` without populating `rowid`.

The `delete_rowid` field uses the current rowid value in the object to
delete. The rowid field is the only field of the object consumed in the
delete method.

The `get_by_rowid` fetches a row from the table using the rowid value.

### CUSTOM queries

The purpose of the CUSTOM queries is to give the user more exposure
to the raw SQL statements. If you need to construct a SELECT statement
with a complex WHERE clause, you can use `CUSTOM-GET`. If you want to
update one or a handful of columns in a row without updating the entire
row, you may use `CUSTOM-UPD`.  If you want to delete using a custom
WHERE clause, use `CUSTOM-DEL`.

TYPEs listed in `CUSTOM-GET` and `CUSTOM-DEL` are listed in
parenthesis and separated by spaces, i.e. `(TEXT DOUBLE)`.

The `where-clause` in `CUSTOM-GET` and `CUSTOM-DEL` is an SQL-compliant
WHERE clause as in the following statement. (Note only the part from
"uid" to the last "?" need be included in `where-clause`:

```SQL
SELECT * FROM users WHERE uid = ? AND lastname LIKE ?
```

`CUSTOM-GET` results in a method of the following form. Note the
arguments are numbered and the TYPEs follow the order in the parenthesis.
Also NOTE you must put exactly as many "?" in the where-clause as you put
types in parenthesis.

```C++
bool get_<get-method-name>(TYPE v1, TYPE v2, [etc]);
```

`CUSTOM-UPD` results in a method of the form:

```C++
bool update_<update-method-name>(void);
```

`CUSTOM-DEL` results in a method of the following form. Note the
arguments are numbered and the TYPEs follow the order in the parenthesis.
Also NOTE you must put exactly as many "?" in the where-clause as you put
types in parenthesis.

```C++
bool delete_<delete-method-name>(TYPE v1, TYPE v2, [etc]);
```

### getting multiple rows

Many query statements may return multiple rows of data.

The first time any `get_` methods are called, the object will contain
the data for the first (or only) row, if the get method returns true.
(If it returns false, no data was returned.)

If you wish to get more rows, you may call:

```C++
bool get_next(void);
```

If this returns true, another row is returned.  Keep calling it until
it returns false to consume all matching rows.

## further customization

Note if customization provided by CUSTOM-GET, CUSTOM-UPD, and CUSTOM-DEL
operators is not sufficient, you may further extend the functionality by
implementing a derived-C++ class from the SQL_TABLE class produced by
this tool. Examine the source code of the generated output for more
information of what you can do. Note the following methods of the SQL_TABLE
class are `protected` so that derived classes may call them:

```C++
sqlite3 *pdb;
bool debug;
sqlite3_stmt * previous_get;
bool get_columns(sqlite3_stmt *pStmt);
```

The `pdb` is the pointer to the database. `debug` is set to true if so
specified in the constructor. Other parts of this class use `debug` to
decide whether to print out the expanded sql statements. `previous_get`
should be written by a query method if you wish to use the `get_next`
method to get more rows.  The `get_columns` method will examine a row
in a prepared statement and extract all column data from that row.
See the autogenerated code for any other `get_` method for an example
of what you can do in your own custom derived classes.

## sample schema file

```
TABLE user
{
	userid		INT64		INDEX	QUERY
	firstname	TEXT
	lastname	TEXT		LIKEQUERY
	mi		TEXT
	ssn		INT		INDEX	QUERY
	balance		DOUBLE
	proto		BLOB

	CUSTOM-GET great_balance (DOUBLE)
		   "balance > ?"

	CUSTOM-GET founders      ()
		   "userid < 100"

	CUSTOM-GET firstlast (TEXT TEXT)
		   "firstname LIKE ? AND lastname LIKE ?"

	CUSTOM-UPD balance  (balance)
	CUSTOM-UPD firstlast (firstname lastname)

	CUSTOM-DEL ssn (INT)
	           "ssn = ?"
}
```

Here is the C++ header for the class created from the above schema:

```C++
class SQL_TABLE_user {
public:
    SQL_TABLE_user(sqlite3 *_pdb, bool _debug = false);
    virtual ~SQL_TABLE_user(void);

    sqlite3_int64 rowid;

    int64_t userid;
    std::string firstname;
    std::string lastname;
    std::string mi;
    int32_t ssn;
    double balance;
    std::string proto;

    bool get_by_userid(int64_t v);
    bool get_by_ssn(int32_t v);
    bool get_by_lastname_like(const std::string &patt);
    bool get_great_balance(double v1);
    bool get_founders(void);
    bool get_firstlast(const std::string & v1, const std::string & v2);
    bool get_next(void);
    bool insert(void); // updates rowid
    bool update(void);
    bool delete_rowid(void); // delete by rowid
    bool update_balance(void);
    bool update_firstlast(void);
    bool delete_ssn(int32_t v1);

    static bool table_create(sqlite3 *pdb);
};
```

Note also the tool emits the following class, which invokes the
`table_create` method for every SQL_TABLE specified in the schema.
This is useful to initialize an empty database with all tables needed:

```C++
class SQL_TABLE_ALL_TABLES {
public:
    static bool table_create_all(sqlite3 *pdb);
};
```

## license ##

This code is under the "Unlicense" license.
For more information, please refer to <http://unlicense.org>.

