
#include "aes_vfs.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

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

int
sqlite3_file_vfs_aes :: init(sqlite3_vfs *_vfs, const char *zName,
                             int flags, int *pOutFlags)
{
    vfs = (sqlite3_vfs_aes *) _vfs;
    pMethods = &io_methods;

    if (zName == NULL)
    {
        printf("PLEASE SUPPORT xOpen(zName == NULL);\n");
        return SQLITE_CANTOPEN;
    }

    printf("vfs: xOpen(name = '%s', flags = 0x%x) ", zName, flags);
    if (pOutFlags)
        *pOutFlags = flags;

    fd = open(zName, O_RDWR | O_CREAT, 0644);
    if ( fd < 0 )
    {
        vfs->last_err = errno;
        printf(" -> ERR %d\n", errno);
        return SQLITE_CANTOPEN;
    }

    printf(" -> fd %d\n", fd);

    return SQLITE_OK;
}

//static
int
sqlite3_file_vfs_aes :: xClose(sqlite3_file *_f)
{
    sqlite3_file_vfs_aes * f = (sqlite3_file_vfs_aes *) _f;
    printf("vfs: xClose fd %d\n", f->fd);
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
    printf("vfs: xRead fd %d pos %lld size %d ",
           f->fd, iOfst, iAmt);

    lseek(f->fd, (off_t) iOfst, SEEK_SET);
    int got = read(f->fd, ptr, iAmt);
    if (got == iAmt)
    {
        printf(" -> %d\n", got);
        return SQLITE_OK;
    }
    else if (got < 0)
    {
        printf(" -> ERR %d\n", errno);
        f->vfs->last_err = errno;
        return SQLITE_IOERR_READ;
    }
    // else
    printf(" -> %d\n", got);
    memset(ptr + got, 0, iAmt - got);
    return SQLITE_IOERR_SHORT_READ;
}

// static
int
sqlite3_file_vfs_aes :: xWrite(sqlite3_file*_f, const void*_ptr,
                               int iAmt, sqlite3_int64 iOfst)
{
    sqlite3_file_vfs_aes * f = (sqlite3_file_vfs_aes *) _f;
    const char *ptr = (const char *)_ptr;
    int put;

    printf("vfs: xWrite fd %d pos %lld size %d ",
           f->fd, iOfst, iAmt);

    lseek(f->fd, (off_t) iOfst, SEEK_SET);
    while (iAmt > 0)
    {
        put = write(f->fd, ptr, iAmt);
        if (put < 0)
        {
            f->vfs->last_err = errno;
            printf("-> ERR %d\n", errno);
            return SQLITE_IOERR_WRITE;
        }
        printf("-> %d ", put);
        iAmt -= put;
        ptr += put;
    }
    printf("\n");
    return SQLITE_OK;
}

// static
int
sqlite3_file_vfs_aes :: xTruncate(sqlite3_file*_f, sqlite3_int64 size)
{
    sqlite3_file_vfs_aes * f = (sqlite3_file_vfs_aes *) _f;
    ftruncate(f->fd, (off_t) size);
    return SQLITE_OK;
}

// static
int
sqlite3_file_vfs_aes :: xSync(sqlite3_file*_f, int flags)
{
    sqlite3_file_vfs_aes * f = (sqlite3_file_vfs_aes *) _f;
    fsync(f->fd);
    return SQLITE_OK;
}

// static
int
sqlite3_file_vfs_aes :: xFileSize(sqlite3_file*_f, sqlite3_int64 *pSize)
{
    sqlite3_file_vfs_aes * f = (sqlite3_file_vfs_aes *) _f;
    struct stat sb;
    if (fstat(f->fd, &sb) < 0)
        return SQLITE_ERROR;

    *pSize = (sqlite3_int64) sb.st_size;
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
#if 0
    sqlite3_file_vfs_aes * f = (sqlite3_file_vfs_aes *) _f;
    printf("xFileControl fd %d op = %d ", f->fd, op);

    switch (op)
    {
    case SQLITE_FCNTL_SIZE_HINT: // 5
    {
        sqlite3_int64 sz = *(sqlite3_int64*) pArg;
        printf("SIZE HINT -> %lld\n", sz);
//        lseek(f->fd, sz-1, SEEK_SET);
//        char c = 0;
//        write(f->fd, &c, 1);
        break;
    }
    case SQLITE_FCNTL_BUSYHANDLER: // 15
        printf("BUSY HANDLER\n");
        break;
    case SQLITE_FCNTL_HAS_MOVED: // 20
        printf("FCNTL HAS MOVED ? \n");
        break;
    case SQLITE_FCNTL_SYNC: // 21
        printf("SYNC\n");
        break;
    case SQLITE_FCNTL_COMMIT_PHASETWO: // 22
        printf("COMMIT PHASE TWO ?\n");
        break;
    case SQLITE_FCNTL_PDB: // 30
        printf("PDB ?\n");
        break;
    default:
        printf(" UNKNOWN XXX \n");
    }
#endif
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
