
#include "sqlite3.h"

struct sqlite3_vfs_aes : public sqlite3_vfs
{
    static void register_vfs(void);
private:
    static  int   my_xOpen(sqlite3_vfs*, const char *zName, sqlite3_file*f,
                           int flags, int *pOutFlags);
    static  int   my_xDelete(sqlite3_vfs*, const char *zName, int syncDir);
    static  int   my_xAccess(sqlite3_vfs*, const char *zName,
                             int flags, int *pResOut);
    static  int   my_xFullPathname(sqlite3_vfs*, const char *zName,
                                   int nOut, char *zOut);
    static void * my_xDlOpen(sqlite3_vfs*, const char *zFilename);
    static void   my_xDlError(sqlite3_vfs*, int nByte, char *zErrMsg);
    static void (*my_xDlSym(sqlite3_vfs*,void*, const char *zSymbol))(void);
    static void   my_xDlClose(sqlite3_vfs*, void*);
    static  int   my_xRandomness(sqlite3_vfs*, int nByte, char *zOut);
    static  int   my_xSleep(sqlite3_vfs*, int microseconds);
    static  int   my_xCurrentTime(sqlite3_vfs*, double*);
    static  int   my_xGetLastError(sqlite3_vfs*, int, char *);
    static  int   my_xCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);
    static  int   my_xSetSystemCall(sqlite3_vfs*, const char *zName,
                                    sqlite3_syscall_ptr);
    static sqlite3_syscall_ptr my_xGetSystemCall(sqlite3_vfs*,
                                                 const char *zName);
    static const char *my_xNextSystemCall(sqlite3_vfs*,
                                          const char *zName);

};

struct sqlite3_file_vfs_aes : public sqlite3_file
{
    // what
};
