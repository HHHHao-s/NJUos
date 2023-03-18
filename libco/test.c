#include "co.h"
#include <stdio.h>
int count=0;


void print(void * arg){
    char *name = arg;
    for(int i=0;i<5;i++){
        printf("%d", count++);
    }
}


int main(){

    struct co *c1 = co_start("a", print, "a");
    struct co *c2 = co_start("b", print, "b");
    co_wait(c1);
    co_wait(c2);

    return 0;
}