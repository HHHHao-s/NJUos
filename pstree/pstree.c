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


void insert_all()
{
    
    struct dirent *d=NULL,*each;

    
    DIR *dir = opendir("/proc");
    if(dir==NULL)handle_error("opendir");
    while((d = readdir(dir)) != NULL){
        
        printf("%s",d->d_name);
    }
    

   
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