#define _GNU_SOURCE
#include <dirent.h>     /* Defines DT_* constants */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <string.h>

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

void *Malloc(size_t size){
    void *p;
    if((p = malloc(size))==NULL) handle_error("malloc");
    return p;
}



typedef struct _tree{
    struct  _tree *bro,*son;

    char name[256];
    int pid;
}Tree;


void Tree_init(Tree *head){
    head->bro=NULL;
    head->son=NULL;
    strcpy(head->name, "init");
    head->pid = 1;
}

Tree *Tree_alloc(char *name, int pid){
    Tree *add = Malloc(sizeof(Tree));
    add->bro = NULL;
    add->son = NULL;
    add->pid = pid;
    strcpy(add->name, name);
    return add;
}


int Tree_insert(Tree *head, int ppid, int pid, char *name){
    if(pid == 1 || head==NULL){
        return -1;
    }
    if(head->pid == ppid){
        if(head->son == NULL){
            head->son = Tree_alloc(name,pid);

            
        }else{
            Tree *p = head->son;
            for(;p->bro;p=p->bro);
            p->bro = Tree_alloc(name,pid);
        }
        return 0;
    }else{
        if(Tree_insert(head->bro, ppid, pid, name)==-1){
            return Tree_insert(head->son, ppid, pid, name);
        }
    }
    return -1;
}




void Tree_print_in(Tree *head, char status[], size_t len){
    
    char buf[512];
    sprintf(buf, "-%s-", head->name);
    size_t buf_len = strlen(buf);

    if(head->son != NULL){
        if(head->son->bro!=NULL){
            printf("%s+",buf);
            status[buf_len] = '|';
        }else{
            printf("buf");
        }
        Tree_print_in(head, status, len + buf_len + 1);
    }
    else{
        printf("%s\n", buf);
        
    }
    if(head->bro!=NULL){
        write(STDOUT_FILENO, status, len);
        fflush(stdout);
        Tree_print_in(head,status, len + buf_len +1);
    }
}

void Tree_print(Tree *head){

    char *status = Malloc(1<<13);
    memset(status, ' ', 1<<13);
    size_t len = 0;
    Tree_print_in(head, status, len);


}




void insert_all(Tree *head)
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
            fscanf(fstatus, "Name: %s\n", pname);

            for(int i=1;i<6;i++){
                if(fgets(buf,256, fstatus)!=NULL)
                ;// printf("%s",buf);
                else handle_error("fgets");
            }
            int ppid=0;
            fscanf(fstatus,"%s %d",buf, &ppid);


            if(fclose(fstatus) == -1) handle_error("fclose");

            printf("%d %s %d\n",pid, pname, ppid);
            Tree_insert(head,ppid,pid,pname);
        }
    }
    
}

int main(int argc, char *argv[])
{

    Tree head;
    Tree_init(&head);

    insert_all(&head);
    // for (int i = 0; i < argc; i++)
    // {
    //     assert(argv[i]);
    //     printf("argv[%d] = %s\n", i, argv[i]);
    // }
    // assert(!argv[argc]);
    return 0;
}