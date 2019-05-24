
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
`obj_sample.h`, `obj_sample.cc`, and `obj_sample.proto` to see what
this tool produces.  `sample_test.cc` shows what code can do with
these classes.

## format of schema file

Schema files may contain comments, preceded by `#` symbols. The entire
remainder of a line beginning with this character is ignored.

```
# this is a comment.
```

Schema files must name a PACKAGE. This puts all code in a C++ namespace,
and is also used for the protobuf file package name.

```
PACKAGE "library"
```

Schema files may include literal data in the proto files, header files,
and source files. Note in headers and source files, the code is outside
the 'namespace' declaration. You may add one if you wish.  You may also
include literal text at both the top and bottom of each file.

```
%PROTOTOP{
  import "sample2.proto";
%}
%PROTOBOTTOM{
%}
```

Tables are demarked with curly braces. The format for a table is thus:

```
TABLE <table-name> VERSION <number>
{
     <column-name>  TYPE  ATTRIBUTES "<field-constraints>"
     <repeat the above for each column>

     SUBTABLE <table-name> <protoid-attribute>

     CONSTRAINT "<constraint-name> <table-constraint>"
     <repeat for each table constraint>

     CUSTOM-GET  <get-method-name>  (<list-of-types>)
                 "<where-clause-for-custom-get>"

     CUSTOM-UPD  <update-method-name>  (<list-of-column-names>)

     CUSTOM-UPDBY <update-method-name>  (<list-of-column-names>)
                 (<list-of-types>) "<where-clause-for-custom-update-by>"

     CUSTOM-DEL  <delete-method-name>  (<list-of-types>)
                 "<where-clause-for-custom-delete>"
}
```

Custom-select statements may be implemented to unify data across
multiple tables.  This is done after all TABLE directives are complete
using a `CUSTOM-SELECT` directive, like one of the two following syntaxes:

```
CUSTOM-SELECT  <custom-select-name>
   (<list-of-table-and-column-names>)
   (<list-of-table-names>)
   (<list-of-types>)
   "<where-clause>"

CUSTOM-SELECT  <custom-select-name>
   (<list-of-table-and-column-names>)
   (<list-of-types>)
   "<FROM clause or JOIN clause> WHERE <where-clause>"
```

A `field-constraint` may include any SQL string that may appear within a column
definition, such as "NOT NULL UNIQUE PRIMARY KEY".

A `table-constraint` may include any SQL string that may appear after column
definitions are complete, such as "<constraint-name> UNIQUE (field, field)".

### keywords

Keywords must be in upper case. Recognized keywords are:

```
protobuf package name : PACKAGE
tables : TABLE VERSION
types : INT INT64 TEXT DOUBLE BLOB BOOL ENUM SUBTABLE
column attributes : INDEX LIKEQUERY QUERY DEFAULT PROTOID FOREIGN AUTOINCR
customs : CUSTOM-GET CUSTOM-UPD CUSTOM-UPDBY CUSTOM-DEL
literal text : %PROTOTOP{ %}    %PROTOBOTTOM{ %}
               %HEADERTOP{ %}   %HEADERBOTTOM{ %}
	       %SOURCETOP{ %}   %SOURCEBOTTOM{ %}
```

The `table-name` will be used to complete a C++ class of the form
`SQL_TABLE_<table-name>`. The column-names declare variables within
the C++ class.

### data types

The TYPE declares the SQL database data type. SQL
data types map to C++ data types as such:

| SQL3GEN | SQLITE3 | C++         | protobuf |
| --------| --------| ----------- | ---------|
| INT     | int     | int32_t     | int32    |
| INT64   | int64   | int64_t     | int64    |
| DOUBLE  | double  | double      | double   |
| TEXT    | text    | std::string | string   |
| BLOB    | blob    | std::string | bytes    |
| BOOL    | int     | bool        | bool     |
| ENUM    | int     | enum        | enum     |

`SUBTABLE` is a special data type to refer to another table which
uses a `FOREIGN` key back to this table. (Especially useful for
1-to-many mappings.)

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

The `TYPE` in the above statement will be the C++ type from the above
table which maps to the SQL type for the column.

NOTE this also populates "rowid" in the class so a successive
`update` will update the same row in the table.

Placing the `LIKEQUERY` keyword instructs the tool to add a method
for retrieving based on an SQL "LIKE" pattern (see SQLITE3 documentation
for more information on the LIKE keyword).  This will add a method
that looks like this:

```C++
bool get_by_<column-name>_like(const std::string &patt);
```

Note the argument of `patt` for a `_like` method will always
be `std::string`.

The `FOREIGN` keyword adds a FOREIGN KEY constraint to the SQL column,
and is required if you wish to use the `SUBTABLE` type. This is useful
for establishing a one-to-many relationship between an entry in one table
and several entries in another table, using a column value to link them.
NOTE if you wish to use `SUBTABLE` in the other table, `QUERY` is
required on the `FOREIGN` field in this table, because the other table's
`get_subtable` method will invoke the query on the foreign key field.

The `AUTOINCR` keyword adds the AUTOINCREMENT constraint to the SQL
column, and removes that column from all INSERT and UPDATE commands.

NOTE the field marked with `AUTOINCR` must also set a "PRIMARY KEY"
constraint (not enforced by `sqlite3gen`, but sqlite3 will throw an
error during table creation if you don't).

NOTE the field marked with `AUTOINCR` should not be written prior
to an `INSERT`. Like `rowid`, this field will be written after the
`INSERT` completes to indicate what value was autogenerated by sqlite3.

To use `FOREIGN` on a field of a `TABLE` you must name the other table
and field this field references:

```
TABLE checkouts VERSION 1
{
    userid INT64 INDEX QUERY UNIQUE FOREIGN user.userid PROTOID 2
    bookid INT64 INDEX QUERY UNIQUE FOREIGN book.bookid PROTOID 3
}
```

You may then use `SUBTABLE` in the other table (`book` table in
this example) to include all `checkouts` entries.

```
TABLE user VERSION 19
{
    SUBTABLE checkouts PROTOID 11
}
```

This adds the following C++ elements to the SQL_TABLE_user class:

```C++
class SQL_TABLE_user {
// NOTE this is only populated by get_subtable_checkouts()
    std::vector<SQL_TABLE_checkouts> checkouts;
// note this assumes foreign key userid is populated;
// returns number of rows fetched.
    int get_subtable_checkouts(void);
}
```

Once the `userid` field of this class is populated with a valid userid,
you may then invoke `get_subtable_checkouts()` to fetch all rows from
the checkouts table matching the given userid, which will populate
the vector.

### DEFAULT attribute

Placing a DEFAULT followed by a number or string will automatically
populate an `init` method in the class:

```C++
void init(void);
```

Calling this method resets all fields in the class to their DEFAULT
values, including setting `rowid` to -1.

NOTE: `BLOB` types cannot be defaulted; a BLOB field always defaults
to zero-length.

`DEFAULT` is required on an `ENUM` type.

### PROTOID attribute

Placing a `PROTOID` followed by an integer will cause a field to be
added to the protobuf definition for this TABLE class's Message.  The
integer specified with PROTOID is the field number in the Message.
(This is to allow forwards and backwards compatibility across versions
of a schema definition, as long as the PROTOID for a field doesn't
change.)

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
arguments are numbered and the `TYPE`s follow the order in the parenthesis.
Also NOTE you must put exactly as many "?" in the where-clause as you put
types in parenthesis.

```C++
bool get_<get-method-name>(TYPE v1, TYPE v2, [etc]);
```

`CUSTOM-UPD` results in a method of the following form. The CUSTOM-UPD
uses `rowid` to select the proper row to perform the update.

```C++
bool update_<update-method-name>(void);
```

`CUSTOM-UPDBY` results in a method of the following form. The
CUSTOM-UPDBY uses the `WHERE` clause to select the proper row(s) to
perform the update. Note this query may update the specified column of
every row which matches the WHERE clause. Note the arguments are
numbered and the TYPEs follow the order in the parenthesis.  Also NOTE
you must put exactly as many "?" in the where-clause as you put types
in parenthesis.

```C++
bool update_by_<update-method-name>(TYPE v1, TYPE v2, [etc]);
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

See the file `sample.schema` in this repository for a sample schema.
See the file `obj_sample.h` and `obj_sample.cc` in this repository
for the C++ header file and source file generated from this schema.

The tool also emits the following class:

```C++
class SQL_TABLE_ALL_TABLES {
public:
    static bool init_all(sqlite3 *pdb, table_version_callback cb);
    static void table_drop_all(sqlite3 *pdb);
};
```

The `init_all` method examines the `tables` table to check whether all
tables specified in the schema exist or not. If they don't exist, they
are created (along with all required indexes). If they exist, it also
checks their version numbers. The `table_version_callback` is invoked
for every table in the schema.  It informs the user the version number
present in the database file, and the version number compiled into
this code (NB: even if they are equal!). This gives the user the
opportunity to perform modifications on the table data structures as
required to make tables conform to new (or old!) versions.

The `table_drop_all` method deletes all tables from the database.

### CUSTOM-SELECT

Once all `TABLE` definitions are complete, you may add `CUSTOM-SELECT`
declarations. These produce separate classes that can perform far more
complex queries than `CUSTOM-GET` queries within a table.

An example follows:

```
CUSTOM-SELECT due_books
   (user.rowid user.firstname user.lastname
    book.rowid book.title checkouts.duedate)
  (user checkouts book)
  (INT INT)
  "checkouts.bookid2 = book.bookid AND checkouts.userid2 = user.userid AND book.bookid > ? AND book.bookid < ? ORDER BY duedate ASC"
```

This will emit a C++ class called `SQL_SELECT_due_books` with the following
data members:

```C++
    sqlite3_int64 user_rowid;
    std::string user_firstname;
    std::string user_lastname;
    sqlite3_int64 book_rowid;
    std::string book_title;
    int64_t checkouts_duedate;
```

(The format of these entries' names is `<tablename>_<fieldname>`. Note
that `rowid` is a valid fieldname, which is a very useful value to have
if you wish to proceed with a `SQL_TABLE_xx` class and call `get_by_rowid`
to get a complete table row.)

This `SQL_SELECT_due_books` also contains two methods:

```C++
    bool get(int32_t v1, int32_t v2);
    bool get_next(void);
```

The `get` method has two arguments matching the `(INT INT)` part of the
schema code above, which corresponds to the two question marks in the
WHERE clause above. Like TABLE classes, the `get_next` method is also
used for iterating through a query result with multiple rows.

## protobuf integration

In the schema file, you may also specify at the top of the file:

```
PROTOPKG "package_name"
```

This tool will now emit a proto file suitable for feeding to google
protobufs `protoc` tool. This proto file will contain a protobuf
Message for each table class containing the fields marked in the `TABLE`
definition with `PROTOID <number>`.  Fields which don't have the `PROTOID`
attribute do not get added to the Message. If a TABLE definition has no
PROTOID attributes on any fields, no Message is emitted for that TABLE.

It will also emit the following functions in the table classes:

```
void CopyToProto(MessageName_m &msg);
void CopyFromProto(const MessageName_m &msg);
```

Note these Copy functions will also marshal data in and out of
any subtable members, if possible.

## debug logging

There is one more function emitted for each table class, of this
form:

```C++
static void register_log_funcs(sql_log_function_t _upd_func,
                               sql_log_function_t _get_func,
                               void *_arg);
```

The user may supply functions for logging the raw SQL statements
generated. This may be useful for debug purposes as well as for
remote database synchronization. This API separates `updates` from
`gets` for this reason. Any SQL generated by these classes which
modify tables will be logged through the `update` function, and
any SQL generated for reading back from the tables will be logged
through the `get` function.

The function in question may use the `sqlite3_expanded_sql` function
to expand the SQL from the `prepared statement`. See `sample_test.cc`
for an example of this.

## license ##

This code is under the "Unlicense" license.
For more information, please refer to <http://unlicense.org>.

