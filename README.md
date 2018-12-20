
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
`obj/sample.h` and `obj/sample.cc` to see what this tool produces.
`sample_test.cc` shows what code can do with these classes.

## license ##

This code is under the "Unlicense" license.
For more information, please refer to <http://unlicense.org>.

