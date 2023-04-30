#include <os.h>
#include <syscall.h>
#include <hashmap.h>
#include "initcode.inc"

// 直接赋值成最大的id
static void assign_id(int *_id){
    static int id_top=1;
    int id;
    while(!(id=atomic_xchg(&id_top, 0)));// acquire lock
    *_id = id++;
    atomic_xchg(&id_top,id);
}

// 记录map，更新pa的引用次数，fork时简单的replay
void log_map(task_t *task, void *vaddr, void *paddr, int prot){
    increase(paddr);
    task->log[task->log_len].va = (uintptr_t)vaddr;
    task->log[task->log_len].pa = (uintptr_t)paddr;
    task->log_len++;
    map(&task->as,vaddr, paddr, prot);
}


static void replay(task_t *taskold, task_t *tasknew){

    for(int i=0;i<taskold->log_len;i++){
        map(&taskold->as, (void *)taskold->log[i].va, NULL, MMAP_NONE);
        map(&taskold->as, (void *)taskold->log[i].va, (void *)taskold->log[i].pa, MMAP_READ);// 修改映射成只读，当pagefault时，查看ref，决定cow或改映射
        
        // 记录是一样的，所以不用log_map
        map(&tasknew->as, (void *)tasknew->log[i].va, (void *)tasknew->log[i].pa, MMAP_READ);// 记录是一样的，所以不用log_map
        increase((void *)taskold->log[i].pa);
    }
}
    

int uinit(task_t *task, const char *name, void (*entry)(void *arg), size_t len){
    
    
    assign_id(&task->id);
    
    char buf[32];
    strncpy(buf,name,32);
    kmt->spin_init(&task->lock, strcat(buf, "user spin lock") );
    task->status = RUNNABLE;
    task->arg = NULL;
    task->log_len=0;
    task->parent = NULL;
    protect(&task->as);

    void *place = pmm->alloc(task->as.pgsize);
    memcpy(place, entry, len);
    void *begin =  task->as.area.start;
    log_map(task, begin,place, MMAP_READ|MMAP_WRITE);

    void *pstack = pmm->alloc(task->as.pgsize);
    uint64_t vrsp = (uint64_t)task->as.area.end;
    void *vstack = (void *)vrsp-task->as.pgsize;
    
    log_map(task, vstack, pstack, MMAP_READ|MMAP_WRITE);

    task->context = ucontext(&task->as ,(Area){.start=&task->fence + 1,.end=task+1},begin);
    task->context->rsp = vrsp; 
    task->entry = begin;

    strncpy(task->name, name, KMT_NAME_SIZE);
    memset(task->fence, 'x', KMT_FENCE_SIZE);
    return task_list_insert(task);

}

static void uproc_init()
{
    vme_init(pmm->alloc, pmm->free);


    uinit(task_alloc(),"user proc", (void (*)(void *))_init, _init_len);

}

static int kputc(task_t *task, char ch)
{
    putch(ch);
    return 0;
}

static int fork(task_t *task)
{
    task_t *tasknew = (task_t *)pmm->alloc(sizeof(task_t));
    memcpy(tasknew, task, sizeof(task_t));
    
    tasknew->parent=task;

    assign_id(&tasknew->id);
    protect(&tasknew->as); // 重新初始化页表
    
    uint64_t offect =  (uint64_t)(task+1) - (uint64_t)task->context;
    tasknew->context = (Context *)((uint64_t)(tasknew+1)- offect);

    tasknew->context->cr3 = NULL;
    tasknew->context->GPRx = 0;
    

    tasknew->status = RUNNABLE;

    replay(task, tasknew);

    task->context->GPRx = tasknew->id;

    task_list_insert(tasknew);


    return 0;// 没有意义
}

static int wait(task_t *task, int *status)
{
    panic("wait");
}

static int exit(task_t *task, int status)
{
    for(int i=0;i<task->log_len;i++){
        decrease((void *)task->log[i].pa);// ref减小到0就会free掉
    }
    unprotect(&task->as);
    if(task->parent){
        task->status = ZOMBIE;
        task->exit_status = status;
    }       
    else{
        task->status = CAN_BE_CLEAR;
        kmt->teardown(task);
    }
    return status;
}

static int kill(task_t *task, int pid)
{
    panic("kill");
}

static void *mmap(task_t *task, void *addr, int length, int prot, int flags)
{
    panic("mmap");
}

static int getpid(task_t *task)
{
    panic("getpid");
}

static int sleep(task_t *task, int seconds)
{
    panic("sleep");
}

static int64_t uptime(task_t *task)
{
    panic("uptime");
}

MODULE_DEF(uproc) = {

    .init = uproc_init,
    .kputc = kputc,
    .fork = fork,
    .wait = wait,
    .exit = exit,
    .kill = kill,
    .mmap = mmap,
    .getpid = getpid,
    .sleep = sleep,
    .uptime = uptime

};
