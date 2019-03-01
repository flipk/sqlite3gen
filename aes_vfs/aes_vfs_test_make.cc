
#include "aes_vfs.h"

int
main()
{
    sqlite3_vfs_aes::register_vfs();
    return 0;
}
