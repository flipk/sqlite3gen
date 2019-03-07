
STRUCTURE SUMMARY
-----------------

sqlite3_vfs_aes : sqlite3_vfs
  PageCipher

sqlite3_file_vfs_aes : sqlite3_file
  DiskCache
      PageCache
          DiskPage[]

STRUCTURE DETAILS
-----------------

SQLITE3 requires a call to sqlite3_vfs_register to supply a pointer to
a vector of functions in a struct sqlite3_vfs.  The class
sqlite3_vfs_aes is derived from sqlite3_vfs and implements this. This
is done in aes_vfs.cc. There is one static sqlite3_vfs_aes object
globally in this file.

The xOpen method must fill out something derived from sqlite3_file to
provide read/write/close/truncate/size/sync/lock/unlock.  This is done
in aes_vfs_file.cc.

The class sqlite3_file_vfs_aes is derived from sqlite3_file.  There is
one of these for every open SQLITE3 database connection.

Encryption is done on page boundaries. Every 4096-byte page has a 32-byte
HMAC.

The implementation of xRead and xWrite interact with a class called
DiskCache. This class deals with read and write calls that may cross
one or more disk page boundaries.

There is one DiskCache per open SQLITE3 database connection, stored
inside the sqlite3_file_vfs_aes object.

DiskCache interacts with a class called PageCache which keeps a cache
of disk pages. The DiskCache contains a hash searchable by page number,
a Least-Recently-Used list which reorders base on usage patterns, and a
full list which keeps track of all pages in the cache for flushing
purposes.

There is one PageCache object per SQLITE3 database connection, stored
within the DiskCache object.

A disk page is stored in a class called DiskPage. It implements read,
write, and flush, and interacts with the encryption/decryption class.

There may be hundreds of DiskPage objects in existence at any time,
stored within a PageCache object.

Encryption and decryption are done in a class called PageCipher. There
is only one of this object globally, stored within the sqlite3_vfs_aes
object. DiskPage invokes PageCipher methods when it wants to encrypt
or decrypt a page.


ENCRYPTION DETAILS
------------------

A password is supplied in a std::string. This is run thru SHA256 to
make a 32-byte "file key".  The AES encryption (in CBC mode) is
initialized using this file key.  The HMAC is also initialized using
this file key.

The 32-byte file key is XOR'd with the "page number" in a specific
pattern (see "make_iv" in PageCipher.cc for details). This 32-byte
unit of data (file key ^ page number) is then run thru SHA256 to make
a 32-byte "page key". The page key is split into two 16-byte halves
which are XOR'd with each other to make the 16-byte CBC IV.

The file is encrypted 4096 bytes (a "page") at a time. The IV is
calculated using the page number and the AES encryption and HMAC
digest is started for each page.  The HMAC algorithm results in 32
extra bytes of HMAC data.  Thus, every SQLITE3 "page" of data is 4096
bytes long but consumes 4128 bytes of space on disk.
