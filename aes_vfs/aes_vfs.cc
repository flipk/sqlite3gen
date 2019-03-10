
#include "aes_vfs.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <time.h>

namespace AES_VFS {

static sqlite3_vfs_aes vfs_aes_obj;

sqlite3_vfs_aes :: sqlite3_vfs_aes(void)
{
    last_err = 0;
}

//static
void
sqlite3_vfs_aes :: register_vfs(void)
{
    vfs_aes_obj.iVersion   = 3;
    vfs_aes_obj.szOsFile   = sizeof(sqlite3_file_vfs_aes);
    vfs_aes_obj.mxPathname = 256;
    vfs_aes_obj.zName      = "aes";
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

    sqlite3_vfs_register( &vfs_aes_obj, /*make default*/ 0 );
}

// static
void
sqlite3_vfs_aes :: setKey(const std::string &password)
{
    vfs_aes_obj.cipher.setKey(password);
}

//static
int
sqlite3_vfs_aes :: my_xOpen(sqlite3_vfs *vfs, const char *zName,
                            sqlite3_file *_f, int flags, int *pOutFlags)
{
    sqlite3_file_vfs_aes * f = (sqlite3_file_vfs_aes *) _f;
    return f->init(vfs, zName, flags, pOutFlags, &vfs_aes_obj.cipher);
}

//static
int
sqlite3_vfs_aes :: my_xDelete(sqlite3_vfs*, const char *zName, int syncDir)
{
    unlink(zName);
    return SQLITE_OK;
}

//static
int
sqlite3_vfs_aes :: my_xAccess(sqlite3_vfs*, const char *zName,
           int flags, int *pResOut)
{
    struct stat sb;
    if (stat(zName, &sb) < 0)
        *pResOut = 0;
    else
        *pResOut = 1;
    return SQLITE_OK;
}

//static
int
sqlite3_vfs_aes :: my_xFullPathname(sqlite3_vfs*, const char *zName,
                 int nOut, char *zOut)
{
    if (zName[0] == '/')
    {
        memset(zOut, 0, nOut);
        strncpy(zOut, zName, nOut-1);
    }
    else
    {
        memset(zOut, 0, nOut);
        char cwd[512];
        cwd[511] = 0;
        getcwd(zOut, 511);
        snprintf(zOut, nOut-1, "%s/%s", cwd, zName);
    }
    return SQLITE_OK;
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
    snprintf(zErrMsg, nByte,
             "Loadable modules are not supported by this VFS");
    zErrMsg[nByte-1] = 0;
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
    return SQLITE_OK;
}

//static
int
sqlite3_vfs_aes :: my_xSleep(sqlite3_vfs*, int microseconds)
{
    sleep(microseconds / 1000000);
    usleep(microseconds % 1000000);
    return microseconds;
}

//static
int
sqlite3_vfs_aes :: my_xCurrentTime(sqlite3_vfs*, double*pTime)
{
    // copied from test_demovfs.c
    time_t t = time(0);
    *pTime = t/86400.0 + 2440587.5;
    return SQLITE_OK;
}

//static
int
sqlite3_vfs_aes :: my_xGetLastError(sqlite3_vfs*_v, int errLen, char *err)
{
    sqlite3_vfs_aes * v = (sqlite3_vfs_aes *) _v;
    snprintf(err, errLen, "%s", strerror(v->last_err));
    return v->last_err;
}

//static
int
sqlite3_vfs_aes :: my_xCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*pTime)
{
    // copied from test_demovfs.c
    time_t t = time(0);
    *pTime = t/86400.0 + 2440587.5;
    return SQLITE_OK;
}

//static
int
sqlite3_vfs_aes :: my_xSetSystemCall(sqlite3_vfs*, const char *zName,
                                     sqlite3_syscall_ptr)
{
    return SQLITE_ERROR;
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

}; // namespace AES_VFS
