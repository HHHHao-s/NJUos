#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dlfcn.h>


void doint(char *line, size_t len){
  int fd = open("/tmp/crepltem.c", O_APPEND|O_WRONLY);

  write(fd,line, len);


  close(fd);

  char *arg[] = {"gcc","-fpic","-shared","/tmp/crepltem.c","-o","/tmp/crepltem.so",NULL };


  if(fork()==0){
    execvp("gcc", arg);
  }
  int status;
  wait(&status);
  if(WEXITSTATUS(status) == EXIT_SUCCESS){
    // printf("%s added\n", line);
  }else{
    // printf("%s wrong\n", line);
  }

}


void docalc(char *line, int index){
  char buf[4096];
  sprintf(buf, "int __warpper_index_%d(){return %s;}\n", index, line);
  doint(buf, strlen(buf));
  void *handle = dlopen("/tmp/crepltem.so", RTLD_NOW);
  if(!handle){
    fprintf(stderr,"%s\n", dlerror());
    return;
  }
  

  dlerror();

  int (*func)();


  char funcname[4096];
  sprintf(funcname,"__warpper_index_%d", index);

  func = (int (*)())dlsym(handle, funcname);


  char *error = dlerror();
  if(error != NULL){
    fprintf(stderr,"%s\n", error);
    return;
  }

  printf("%s = %d\n", line, func());

  dlclose(handle);


}


int main(int argc, char *argv[]) {
  static char line[4096];
  
  int fd = open("/tmp/crepltem.c", O_TRUNC|O_CREAT|O_WRONLY);
  write(fd, "int main(){return 0;}\n", strlen("int main(){return 0;}\n"));
  close(fd);
  int count=0; // count of calculate instructions
  while (1) {
    printf("crepl> ");
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) {
      break;
    }
    if(line[0]=='i' && line[1] =='n' && line[2] == 't'){
      doint(line, strlen(line));
    }else{
      docalc(line, count++);
    }
  }
}
