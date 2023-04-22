#include <os.h>
#include <syscall.h>

#include "initcode.inc"


int ucreate(task_t *task, const char *name, void (*entry)(void *arg), void *arg){
    panic("ucreate");
    char buf[32];
    strncpy(buf,name,32);
    kmt->spin_init(&task->lock, strcat(buf, " spin lock") );
    task->status = RUNNABLE;
    task->entry = entry;
    task->arg = arg;
    task->context = ucontext(&task->as ,(Area){.start=&task->fence + 1,.end=task+1},entry);
    strncpy(task->name, name, KMT_NAME_SIZE);
    memset(task->fence, 'x', KMT_FENCE_SIZE);
    return task_list_insert(task);

}

static void uproc_init()
{
    vme_init(pmm->alloc, pmm->free);
    void(*p)() = (void *)_init;
    p();

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
