#include "co.h"
#include <stdlib.h>
#include <setjmp.h>
#include <sys/types.h>
#include <stdint.h>

static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg)
{
    asm volatile(
#if __x86_64__
        "movq %0, %%rsp; movq %2, %%rdi; jmp *%1"
        :
        : "b"((uintptr_t)sp), "d"(entry), "a"(arg)
        : "memory"
#else
        "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
        :
        : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg)
        : "memory"
#endif
    );
}

#define STACKSIZE 65528
enum co_status
{
    CO_NEW = 1, // 新创建，还未执行过
    CO_RUNNING, // 已经执行过
    CO_WAITING, // 在 co_wait 上等待
    CO_DEAD,    // 已经结束，但还未释放资源
};

struct co
{
    
    __uint8_t stack[STACKSIZE]; // 协程的堆栈
    void * retfun;              // 协程执行完后要到的函数
    const char *name;
    void (*func)(void *); // co_start 指定的入口地址和参数
    void *arg;

    enum co_status status;      // 协程的状态
    struct co *waiter;          // 是否有其他协程在等待当前协程
    jmp_buf jb;                 // 寄存器现场 (setjmp.h)
    
};
struct co *current;
struct _q
{
    struct co *array[128];
    size_t size;
};

struct _q q = {.size =0};


static struct co * pick(){
    // pick one routinue to run

    int waiter=0;
    for(int i=0;i<q.size;i++){
        if(q.array[i]->status == CO_WAITING){
            waiter = i;
            continue;
        }
        if(q.array[i]->status != CO_DEAD && current!=q.array[i]){ // only not dead or waiting

            return q.array[i];
        }
    }
    return q.array[waiter];
}


static void push(struct co *co_r)
{
    q.array[q.size++] = co_r;
}

static void delete(struct co *co_r)
{
    if (q.size < 1)
    {
        return;
    }
    int i = 0;

    for (; i < q.size; i++)
    {
        if (co_r == q.array[i])
        {
            break;
        }
    }
    for (int j = i + 1; j < q.size; j++)
    {
        q.array[i] = q.array[j];
    }
    q.size--;
}


struct co *co_start(const char *name, void (*func)(void *), void *arg)
{
    struct co *newco = malloc(sizeof(struct co));
    newco->name = name;
    newco->func = func;
    newco->arg = arg;
    newco->status = CO_NEW;
    newco->waiter = NULL;

    push(newco);

    return newco;
}

void co_wait(struct co *co)
{
    while (1){
        if (co->status == CO_DEAD)
        {
            delete(co);
            free(co);
            break;
        }
        else
        {
            co->waiter = current;
            current->status = CO_WAITING;
            co_yield();
        }
    }
    
}

static void co_finish()
{
    current->status = CO_DEAD;
    co_yield();
}

void co_yield()
{
    int val = setjmp(current->jb);
    if (val == 0)
    {
        // pick one routinue to run
        struct co * next=pick();
        if(next->status == CO_NEW){
            next->status = CO_RUNNING;
            next->retfun = co_finish; 
            stack_switch_call(&next->stack[STACKSIZE],next->func, (uintptr_t)next->arg); // 数据结构在堆上申请，低地址是结构的第一个参数，而栈是向下增长，所以要用高地址作为栈顶
        }
        else{
            longjmp(next->jb, 1);
        }
    }
    else
    {
        ;
    }
}

void  __attribute__((constructor)) before();
void before(){
    current = co_start("main", NULL, NULL);
    current->status = CO_RUNNING;
    
}
