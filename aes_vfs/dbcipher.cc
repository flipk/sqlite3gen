
#include <string>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "PageCipher.h"

void
usage(void)
{
    printf("usage: dbcipher e <password> <infile> <outfile>\n"
           "       dbcipher d <password> <infile> <outfile>\n");
}



int
main(int argc, char ** argv)
{
    if (argc != 5)
    {
        usage();
        return 1;
    }

    bool encrypt = false;

    if (argv[1][0] == 'e')
        encrypt = true;
    else if (argv[1][0] != 'd')
    {
        usage();
        return 1;
    }

    std::string password(argv[2]);
    std::string infile(argv[3]);
    std::string outfile(argv[4]);

    int in_fd = open(infile.c_str(), O_RDONLY);
    if (in_fd < 0)
    {
        printf("unable to open input file: %s\n", strerror(errno));
        return 1;
    }

    int out_fd = open(outfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd < 0)
    {
        printf("unable to open output file: %s\n", strerror(errno));
        return 1;
    }

    AES_VFS::PageCipher  pc;
    pc.setKey(password);

    static const int PAGE_SIZE_DISK = AES_VFS::PageCipher::PAGE_SIZE_DISK;
    static const int PAGE_SIZE = AES_VFS::PageCipher::PAGE_SIZE;

    uint8_t disk_buffer[PAGE_SIZE_DISK];
    uint8_t page_buffer[PAGE_SIZE];

    uint32_t pgno = 0;

    if (encrypt)
    {
        while (1)
        {
            int cc = read(in_fd, (void*) page_buffer, PAGE_SIZE);
            if (cc != PAGE_SIZE)
            {
                if (cc < 0)
                {
                    printf("read error: %s\n", strerror(errno));
                    break;
                }
                if (cc == 0)
                    break;
                memset(page_buffer + cc, 0, PAGE_SIZE - cc);
            }
            pc.encrypt_page(pgno, disk_buffer, page_buffer);
            cc = write(out_fd, (void*) disk_buffer, PAGE_SIZE_DISK);
            if (cc != PAGE_SIZE_DISK)
                break;
            pgno ++;
        }
    }
    else
    {
        while (1)
        {
            int cc = read(in_fd, (void*) disk_buffer, PAGE_SIZE_DISK);
            if (cc != PAGE_SIZE_DISK)
                break;
            pc.decrypt_page(pgno, page_buffer, disk_buffer);
            cc = write(out_fd, (void*) page_buffer, PAGE_SIZE);
            if (cc != PAGE_SIZE)
                break;
            pgno ++;
        }
    }

    close(in_fd);
    close(out_fd);

    printf("read %d input pages\n", pgno);

    return 0;
}
