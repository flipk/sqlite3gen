
#include "DiskPage.h"

namespace AES_VFS {

DiskPage :: DiskPage(int _fd, uint32_t _pg, PageCipher * _cipher)
    : cipher(_cipher), fd(_fd), pageNumber(_pg)
{
    dirty = false;
    read();
}

DiskPage :: ~DiskPage(void)
{
    flush();
}

void
DiskPage :: flush(void)
{
    if (dirty)
        write();
    dirty = false;
}

bool
DiskPage :: write(void)
{
    if (!seek())
        return false;

    uint8_t disk_buffer[PAGE_SIZE_DISK];
    cipher->encrypt_page(pageNumber, disk_buffer, buffer);
    int r = ::write(fd, (void*) disk_buffer, PAGE_SIZE_DISK);
    if (r < 0)
        return false;
    return true;
}

bool
DiskPage :: read(void)
{
    if (!seek())
        return false;

    uint8_t disk_buffer[PAGE_SIZE_DISK];
    int r = ::read(fd, (void*) disk_buffer, PAGE_SIZE_DISK);
    if (r < 0)
        return false;
    if (r != PAGE_SIZE_DISK)
        memset(buffer, 0, PAGE_SIZE);
    else
        cipher->decrypt_page(pageNumber, buffer, disk_buffer);

    dirty = false;
    return true;
}

}; // namespace AES_VFS
