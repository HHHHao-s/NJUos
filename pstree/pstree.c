#define _GNU_SOURCE
#include <dirent.h>     /* Defines DT_* constants */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

struct linux_dirent {
    long           d_ino;
    off_t          d_off;
    unsigned short d_reclen;
    char           d_name[];
};

#define BUF_SIZE 1024


void insert_all()
{
    
    struct dirent *d=NULL;

    
    DIR *dir = opendir("/proc");
    d = readdir(dir);

    write(STDOUT_FILENO, d, 100000);

   
}

int main(int argc, char *argv[])
{
    insert_all();
    // for (int i = 0; i < argc; i++)
    // {
    //     assert(argv[i]);
    //     printf("argv[%d] = %s\n", i, argv[i]);
    // }
    // assert(!argv[argc]);
    return 0;
}