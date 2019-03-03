
#include "aes_vfs.h"

#include <stdio.h>

static sqlite3_vfs_aes vfs_aes_obj;

//static
void
sqlite3_vfs_aes :: register_vfs(void)
{
    vfs_aes_obj.iVersion   = 3;
    vfs_aes_obj.szOsFile   = sizeof(sqlite3_file_vfs_aes);
    vfs_aes_obj.mxPathname = 1024;
    vfs_aes_obj.zName      = "aes_vfs";
    vfs_aes_obj.pAppData   = NULL; // what?

    vfs_aes_obj.xOpen             = &sqlite3_vfs_aes::my_xOpen;
    vfs_aes_obj.xDelete           = &sqlite3_vfs_aes::my_xDelete;
    vfs_aes_obj.xAccess           = &sqlite3_vfs_aes::my_xAccess;
    vfs_aes_obj.xFullPathname     = &sqlite3_vfs_aes::my_xFullPathname;
    vfs_aes_obj.xDlOpen           = &sqlite3_vfs_aes::my_xDlOpen;
    vfs_aes_obj.xDlError          = &sqlite3_vfs_aes::my_xDlError;
    vfs_aes_obj.xDlSym            = &sqlite3_vfs_aes::my_xDlSym;
    vfs_aes_obj.xDlClose          = &sqlite3_vfs_aes::my_xDlClose;
    vfs_aes_obj.xRandomness       = &sqlite3_vfs_aes::my_xRandomness;
    vfs_aes_obj.xSleep            = &sqlite3_vfs_aes::my_xSleep;
    vfs_aes_obj.xCurrentTime      = &sqlite3_vfs_aes::my_xCurrentTime;
    vfs_aes_obj.xGetLastError     = &sqlite3_vfs_aes::my_xGetLastError;
    vfs_aes_obj.xCurrentTimeInt64 = &sqlite3_vfs_aes::my_xCurrentTimeInt64;
    vfs_aes_obj.xSetSystemCall    = &sqlite3_vfs_aes::my_xSetSystemCall;
    vfs_aes_obj.xGetSystemCall    = &sqlite3_vfs_aes::my_xGetSystemCall;
    vfs_aes_obj.xNextSystemCall   = &sqlite3_vfs_aes::my_xNextSystemCall;

    sqlite3_vfs_register( &vfs_aes_obj, 0 );
}

//static
int
sqlite3_vfs_aes :: my_xOpen(sqlite3_vfs *vfs, const char *zName,
                            sqlite3_file *_f, int flags, int *pOutFlags)
{
    sqlite3_file_vfs_aes * f = (sqlite3_file_vfs_aes *) _f;
    f->init(vfs);

    return 1;
}

//static
int
sqlite3_vfs_aes :: my_xDelete(sqlite3_vfs*, const char *zName, int syncDir)
{
    return 1;
}

//static
int
sqlite3_vfs_aes :: my_xAccess(sqlite3_vfs*, const char *zName,
           int flags, int *pResOut)
{
    return 1;
}

//static
int
sqlite3_vfs_aes :: my_xFullPathname(sqlite3_vfs*, const char *zName,
                 int nOut, char *zOut)
{
    return 1;
}

//static
void *
sqlite3_vfs_aes :: my_xDlOpen(sqlite3_vfs*, const char *zFilename)
{
    return NULL;
}

//static
void
sqlite3_vfs_aes :: my_xDlError(sqlite3_vfs*, int nByte, char *zErrMsg)
{
}

//static
void
(*sqlite3_vfs_aes :: my_xDlSym(sqlite3_vfs*,void*, const char *zSymbol))(void)
{
    return NULL;
}

//static
void
sqlite3_vfs_aes :: my_xDlClose(sqlite3_vfs*, void*)
{
}

//static
int
sqlite3_vfs_aes :: my_xRandomness(sqlite3_vfs*, int nByte, char *zOut)
{
    return 1;
}

//static
int
sqlite3_vfs_aes :: my_xSleep(sqlite3_vfs*, int microseconds)
{
    return 1;
}

//static
int
sqlite3_vfs_aes :: my_xCurrentTime(sqlite3_vfs*, double*)
{
    return 1;
}

//static
int
sqlite3_vfs_aes :: my_xGetLastError(sqlite3_vfs*, int, char *)
{
    return 1;
}

//static
int
sqlite3_vfs_aes :: my_xCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*)
{
    return 1;
}

//static
int
sqlite3_vfs_aes :: my_xSetSystemCall(sqlite3_vfs*, const char *zName,
                                     sqlite3_syscall_ptr)
{
    return 1;
}

//static
sqlite3_syscall_ptr
sqlite3_vfs_aes :: my_xGetSystemCall(sqlite3_vfs*,
                                     const char *zName)
{
    return NULL;
}

//static
const char *
sqlite3_vfs_aes :: my_xNextSystemCall(sqlite3_vfs*,
                                      const char *zName)
{
    return NULL;
}
