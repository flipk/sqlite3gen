
#include "aes_vfs.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace AES_VFS {

//static
const sqlite3_io_methods sqlite3_file_vfs_aes::io_methods =
{
    3,
    &sqlite3_file_vfs_aes::xClose,
    &sqlite3_file_vfs_aes::xRead,
    &sqlite3_file_vfs_aes::xWrite,
    &sqlite3_file_vfs_aes::xTruncate,
    &sqlite3_file_vfs_aes::xSync,
    &sqlite3_file_vfs_aes::xFileSize,
    &sqlite3_file_vfs_aes::xLock,
    &sqlite3_file_vfs_aes::xUnlock,
    &sqlite3_file_vfs_aes::xCheckReservedLock,
    &sqlite3_file_vfs_aes::xFileControl,
    &sqlite3_file_vfs_aes::xSectorSize,
    &sqlite3_file_vfs_aes::xDeviceCharacteristics,
    &sqlite3_file_vfs_aes::xShmMap,
    &sqlite3_file_vfs_aes::xShmLock,
    &sqlite3_file_vfs_aes::xShmBarrier,
    &sqlite3_file_vfs_aes::xShmUnmap,
    &sqlite3_file_vfs_aes::xFetch,
    &sqlite3_file_vfs_aes::xUnfetch
};

int
sqlite3_file_vfs_aes :: init(sqlite3_vfs *_vfs, const char *zName,
                             int flags, int *pOutFlags, PageCipher *cipher)
{
    vfs = (sqlite3_vfs_aes *) _vfs;
    pMethods = &io_methods;
    if (zName == NULL)
    {
        printf("PLEASE SUPPORT xOpen(zName == NULL);\n");
        return SQLITE_CANTOPEN;
    }
    if (pOutFlags)
        *pOutFlags = flags;
    fd = open(zName, O_RDWR | O_CREAT, 0644);
    if ( fd < 0 )
    {
        vfs->last_err = errno;
        return SQLITE_CANTOPEN;
    }
    dc = new AES_VFS::DiskCache(fd, 5000, cipher);
    return SQLITE_OK;
}

//static
int
sqlite3_file_vfs_aes :: xClose(sqlite3_file *_f)
{
    sqlite3_file_vfs_aes * f = (sqlite3_file_vfs_aes *) _f;
    delete f->dc;
    close(f->fd);
    return SQLITE_OK;
}

// static
int
sqlite3_file_vfs_aes :: xRead(sqlite3_file*_f, void*_ptr, int iAmt,
                              sqlite3_int64 iOfst)
{
    sqlite3_file_vfs_aes * f = (sqlite3_file_vfs_aes *) _f;
    char *ptr = (char *)_ptr;
    int got = f->dc->read((off_t) iOfst, (uint8_t*)_ptr, iAmt);
    if (got == iAmt)
    {
        return SQLITE_OK;
    }
    else if (got < 0)
    {
        f->vfs->last_err = errno;
        return SQLITE_IOERR_READ;
    }
    memset(ptr + got, 0, iAmt - got);
    return SQLITE_IOERR_SHORT_READ;
}

// static
int
sqlite3_file_vfs_aes :: xWrite(sqlite3_file*_f, const void*_ptr,
                               int iAmt, sqlite3_int64 iOfst)
{
    sqlite3_file_vfs_aes * f = (sqlite3_file_vfs_aes *) _f;
    f->dc->write((off_t) iOfst, (uint8_t*)_ptr, iAmt);
    return SQLITE_OK;
}

// static
int
sqlite3_file_vfs_aes :: xTruncate(sqlite3_file*_f, sqlite3_int64 size)
{
    sqlite3_file_vfs_aes * f = (sqlite3_file_vfs_aes *) _f;
    f->dc->truncate((off_t) size);
    return SQLITE_OK;
}

// static
int
sqlite3_file_vfs_aes :: xSync(sqlite3_file*_f, int flags)
{
    sqlite3_file_vfs_aes * f = (sqlite3_file_vfs_aes *) _f;
    if (f->vfs->sync_mode != 0)
    {
        f->dc->flush();
        fsync(f->fd);
    }
    return SQLITE_OK;
}

// static
int
sqlite3_file_vfs_aes :: xFileSize(sqlite3_file*_f, sqlite3_int64 *pSize)
{
    sqlite3_file_vfs_aes * f = (sqlite3_file_vfs_aes *) _f;
    *pSize = f->dc->getFileSize();
    return SQLITE_OK;
}

// static
int
sqlite3_file_vfs_aes :: xLock(sqlite3_file*_f, int)
{
    return SQLITE_OK;
}

// static
int
sqlite3_file_vfs_aes :: xUnlock(sqlite3_file*_f, int)
{
    return SQLITE_OK;
}

// static
int
sqlite3_file_vfs_aes :: xCheckReservedLock(sqlite3_file*_f, int *pResOut)
{
    *pResOut = 0;
    return SQLITE_OK;
}

// static
int
sqlite3_file_vfs_aes :: xFileControl(sqlite3_file*_f, int op, void *pArg)
{
    sqlite3_file_vfs_aes * f = (sqlite3_file_vfs_aes *) _f;
    switch (op)
    {
    case SQLITE_FCNTL_PRAGMA:
    {
        char ** args = (char **) pArg;
        std::string  pragma(args[1]);
        std::string  value(args[2]);
        if (pragma == "journal_mode")
        {
            if (value == "off")
            {
                printf("turning journal mode OFF\n");
                f->vfs->journal_mode = false;
            }
            else
            {
                printf("turning journal mode ON\n");
                f->vfs->journal_mode = true;
            }
        }
        else if (pragma == "synchronous")
        {
            int val = atoi(value.c_str());
            printf("setting sync to %d\n", val);
            f->vfs->sync_mode = val;
        }
        break;
    }
    case SQLITE_FCNTL_BUSYHANDLER:
        break;
    case SQLITE_FCNTL_MMAP_SIZE:
        break;
    case SQLITE_FCNTL_HAS_MOVED:
        break;
    case SQLITE_FCNTL_COMMIT_PHASETWO:
        break;
    case SQLITE_FCNTL_PDB:
        break;
    }

    return SQLITE_OK;
}

// static
int
sqlite3_file_vfs_aes :: xSectorSize(sqlite3_file*)
{
    return 4096;
}

// static
int
sqlite3_file_vfs_aes :: xDeviceCharacteristics(sqlite3_file*)
{
    return
        SQLITE_IOCAP_ATOMIC4K |
        SQLITE_IOCAP_SAFE_APPEND |
        SQLITE_IOCAP_SEQUENTIAL |
        SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN;
}

// static
int
sqlite3_file_vfs_aes :: xShmMap(sqlite3_file*, int iPg,
                                int pgsz, int, void volatile**)
{
    return SQLITE_ERROR;
}

// static
int
sqlite3_file_vfs_aes :: xShmLock(sqlite3_file*, int offset,
                                 int n, int flags)
{
    return SQLITE_ERROR;
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
    return SQLITE_ERROR;
}

// static
int
sqlite3_file_vfs_aes :: xFetch(sqlite3_file*, sqlite3_int64 iOfst,
                               int iAmt, void **pp)
{
    return SQLITE_ERROR;
}

// static
int
sqlite3_file_vfs_aes :: xUnfetch(sqlite3_file*, sqlite3_int64 iOfst,
                                 void *p)
{
    return SQLITE_ERROR;
}

}; // namespace AES_VFS
