
#include "sqlite3.h"
#include "DiskCache.h"

namespace AES_VFS {

struct sqlite3_vfs_aes : public sqlite3_vfs
{
    sqlite3_vfs_aes(void);
    static void register_vfs(void);
    static void setKey(const std::string &password);
    int         last_err;
    PageCipher  cipher;
    int         sync_mode;
    bool        journal_mode;
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
    int init(sqlite3_vfs *_vfs, const char *zName,
             int flags, int *pOutFlags, PageCipher *cipher);
private:
    int fd;
    AES_VFS::DiskCache *dc;
    sqlite3_vfs_aes * vfs;
    static const sqlite3_io_methods io_methods;
    static int xClose(sqlite3_file *);
    static int xRead(sqlite3_file*, void*, int iAmt, sqlite3_int64 iOfst);
    static int xWrite(sqlite3_file*, const void*, int iAmt,
                      sqlite3_int64 iOfst);
    static int xTruncate(sqlite3_file*, sqlite3_int64 size);
    static int xSync(sqlite3_file*, int flags);
    static int xFileSize(sqlite3_file*, sqlite3_int64 *pSize);
    static int xLock(sqlite3_file*, int);
    static int xUnlock(sqlite3_file*, int);
    static int xCheckReservedLock(sqlite3_file*, int *pResOut);
    static int xFileControl(sqlite3_file*, int op, void *pArg);
    static int xSectorSize(sqlite3_file*);
    static int xDeviceCharacteristics(sqlite3_file*);
    static int xShmMap(sqlite3_file*, int iPg, int pgsz, int,
                       void volatile**);
    static int xShmLock(sqlite3_file*, int offset, int n, int flags);
    static void xShmBarrier(sqlite3_file*);
    static int xShmUnmap(sqlite3_file*, int deleteFlag);
    static int xFetch(sqlite3_file*, sqlite3_int64 iOfst,
                      int iAmt, void **pp);
    static int xUnfetch(sqlite3_file*, sqlite3_int64 iOfst, void *p);
};

}; // namespace AES_VFS
