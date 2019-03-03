
#include "aes_vfs.h"

#include <stdio.h>

//static
const sqlite3_io_methods sqlite3_file_vfs_aes::io_methods =
{
    .iVersion               = 3,
    .xClose                 = &sqlite3_file_vfs_aes::xClose,
    .xRead                  = &sqlite3_file_vfs_aes::xRead,
    .xWrite                 = &sqlite3_file_vfs_aes::xWrite,
    .xTruncate              = &sqlite3_file_vfs_aes::xTruncate,
    .xSync                  = &sqlite3_file_vfs_aes::xSync,
    .xFileSize              = &sqlite3_file_vfs_aes::xFileSize,
    .xLock                  = &sqlite3_file_vfs_aes::xLock,
    .xUnlock                = &sqlite3_file_vfs_aes::xUnlock,
    .xCheckReservedLock     = &sqlite3_file_vfs_aes::xCheckReservedLock,
    .xFileControl           = &sqlite3_file_vfs_aes::xFileControl,
    .xSectorSize            = &sqlite3_file_vfs_aes::xSectorSize,
    .xDeviceCharacteristics = &sqlite3_file_vfs_aes::xDeviceCharacteristics, 
    .xShmMap                = &sqlite3_file_vfs_aes::xShmMap,
    .xShmLock               = &sqlite3_file_vfs_aes::xShmLock,
    .xShmBarrier            = &sqlite3_file_vfs_aes::xShmBarrier,
    .xShmUnmap              = &sqlite3_file_vfs_aes::xShmUnmap,
    .xFetch                 = &sqlite3_file_vfs_aes::xFetch,
    .xUnfetch               = &sqlite3_file_vfs_aes::xUnfetch
};

void
sqlite3_file_vfs_aes :: init(sqlite3_vfs *_vfs)
{
    vfs = _vfs;
    pMethods = &io_methods;

}

//static
int
sqlite3_file_vfs_aes :: xClose(sqlite3_file *_f)
{
    sqlite3_file_vfs_aes * f = cast(_f);
    
    return 1;
}

// static
int
sqlite3_file_vfs_aes :: xRead(sqlite3_file*, void*, int iAmt,
                              sqlite3_int64 iOfst)
{
    return 1;
}

// static
int
sqlite3_file_vfs_aes :: xWrite(sqlite3_file*, const void*,
                               int iAmt, sqlite3_int64 iOfst)
{
    return 1;
}

// static
int
sqlite3_file_vfs_aes :: xTruncate(sqlite3_file*, sqlite3_int64 size)
{
    return 1;
}

// static
int
sqlite3_file_vfs_aes :: xSync(sqlite3_file*, int flags)
{
    return 1;
}

// static
int
sqlite3_file_vfs_aes :: xFileSize(sqlite3_file*, sqlite3_int64 *pSize)
{
    return 1;
}

// static
int
sqlite3_file_vfs_aes :: xLock(sqlite3_file*, int)
{
    return 1;
}

// static
int
sqlite3_file_vfs_aes :: xUnlock(sqlite3_file*, int)
{
    return 1;
}

// static
int
sqlite3_file_vfs_aes :: xCheckReservedLock(sqlite3_file*, int *pResOut)
{
    return 1;
}

// static
int
sqlite3_file_vfs_aes :: xFileControl(sqlite3_file*, int op, void *pArg)
{
    return 1;
}

// static
int
sqlite3_file_vfs_aes :: xSectorSize(sqlite3_file*)
{
    return 1;
}

// static
int
sqlite3_file_vfs_aes :: xDeviceCharacteristics(sqlite3_file*)
{
    return 1;
}

// static
int
sqlite3_file_vfs_aes :: xShmMap(sqlite3_file*, int iPg,
                                int pgsz, int, void volatile**)
{
    return 1;
}

// static
int
sqlite3_file_vfs_aes :: xShmLock(sqlite3_file*, int offset,
                                 int n, int flags)
{
    return 1;
}

//static
void
sqlite3_file_vfs_aes :: xShmBarrier(sqlite3_file*)
{
}

// static
int
sqlite3_file_vfs_aes :: xShmUnmap(sqlite3_file*, int deleteFlag)
{
    return 1;
}

// static
int
sqlite3_file_vfs_aes :: xFetch(sqlite3_file*, sqlite3_int64 iOfst,
                               int iAmt, void **pp)
{
    return 1;
}

// static
int
sqlite3_file_vfs_aes :: xUnfetch(sqlite3_file*, sqlite3_int64 iOfst,
                                 void *p)
{
    return 1;
}
