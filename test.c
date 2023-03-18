#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

int main(){

    jmp_buf jb;
    setjmp(jb);
    printf("now");
    longjmp(jb,1);

    return 0;
}