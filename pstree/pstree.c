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
    
    struct dirent *d=NULL;

    
    DIR *dir = opendir("/proc");
    if(dir==NULL)handle_error("opendir");
    while((d = readdir(dir)) != NULL){
        char *pid_c = d->d_name;
        // printf("%s\n", pid_c);
        int pid=0;

        for(;*pid_c;pid_c++){
            if(*pid_c>'0' && *pid_c<'9') {
                pid = pid*10 + *pid_c-'0';
                // printf("%d\n",pid);
            }
            
            else break;
        }
        
        if(*pid_c == '\0'){
            // struct dirent *dpid;
            char status_name[256], buf[256];
            sprintf( status_name,"/proc/%d/status", pid);
            
            FILE *fstatus = fopen(status_name, "r");
            if(!fstatus)handle_error("open status");
            
            char pname[256];
            fscanf(fstatus, "Name: %s", pname);

            for(int i=1;i<6;i++){
                if(fgets(buf,256, fstatus)!=NULL)
                ;// printf("%s",buf);
                else handle_error("fgets");
            }
            int ppid=0;
            fscanf(fstatus,"%s %d",buf, &ppid);


            if(fclose(fstatus) == -1) handle_error("fclose");

            printf("%s %d", pname, ppid);

        }
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