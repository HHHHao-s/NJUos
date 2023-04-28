#include <os.h>
#include <syscall.h>

#include "initcode.inc"


int uinit(task_t *task, const char *name, void (*entry)(void *arg), size_t len){
    // panic("ucreate");

    task->id = 1;
    char buf[32];
    strncpy(buf,name,32);
    kmt->spin_init(&task->lock, strcat(buf, "user spin lock") );
    task->status = RUNNABLE;

    task->arg = NULL;

    protect(&task->as);

    void *place = pmm->alloc(task->as.pgsize);
    memcpy(place, entry, len);
    void *begin =  task->as.area.start;
    map(&task->as, begin,place, MMAP_READ|MMAP_WRITE);

    void *pstack = pmm->alloc(task->as.pgsize);
    uint64_t vrsp = (uint64_t)task->as.area.end;
    void *vstack = (void *)vrsp-task->as.pgsize;
    
    map(&task->as, vstack, pstack, MMAP_READ|MMAP_WRITE);

    task->context = ucontext(&task->as ,(Area){.start=&task->fence + 1,.end=task+1},begin);
    task->context->rsp = vrsp; // 防止指针超出栈
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
    panic("fork");

}

static int wait(task_t *task, int *status)
{
    panic("wait");
}

static int exit(task_t *task, int status)
{
    panic("exit");
    unprotect(&task->as);
    kmt->teardown(task);
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
