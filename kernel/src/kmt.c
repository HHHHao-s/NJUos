#include <os.h>

// void (*init)();
// int  (*create)(task_t *task, const char *name, void (*entry)(void *arg), void *arg);
// void (*teardown)(task_t *task);
// void (*spin_init)(spinlock_t *lk, const char *name);
// void (*spin_lock)(spinlock_t *lk);
// void (*spin_unlock)(spinlock_t *lk);
// void (*sem_init)(sem_t *sem, const char *name, int value);
// void (*sem_wait)(sem_t *sem);
// void (*sem_signal)(sem_t *sem);

static void kmt_spin_init(spinlock_t *lk, const char *name);
static void kmt_spin_lock(spinlock_t *lk);
static void kmt_spin_unlock(spinlock_t *lk);
static void kmt_sem_init(sem_t *sem, const char *name, int value);
static void kmt_sem_wait(sem_t *sem);
static void kmt_sem_signal(sem_t *sem);
static void kmt_init();
static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg);
static void kmt_teardown(task_t *task);
int task_list_insert(task_t *task);
void task_list_delete(task_t *task);


static void kmt_spin_init(spinlock_t *lk, const char *name){
    lk->lock = 1;
    strncpy(lk->name,name,KMT_NAME_SIZE);
}

static void kmt_spin_lock(spinlock_t *lk){
    
    atom_printf("%s spin acquire\n", lk->name);
    

    while(!atomic_xchg(&lk->lock,0)){yield();};

    
    atom_printf("%s spin got\n", lk->name);
    
}

static void kmt_spin_unlock(spinlock_t *lk){
    panic_on(lk->lock!=0,"释放的自旋锁不是0");
    atomic_xchg(&lk->lock, 1);
    printf("%s spin release\n", lk->name);
}

static void kmt_sem_init(sem_t *sem, const char *name, int value){
    char buf[32];
    sprintf(buf,"%s bin_lock", name);
    kmt_spin_init(&sem->bin_lock,buf);
    strncpy(sem->name, name, KMT_NAME_SIZE);
    sem->val = value;
}


static void kmt_sem_wait(sem_t *sem){
    atom_printf("%s require\n", sem->name);
    while(1){
        kmt_spin_lock(&sem->bin_lock);
        if(sem->val>0){
            sem->val--;
            kmt_spin_unlock(&sem->bin_lock);
            break;
        }
        
        kmt_spin_unlock(&sem->bin_lock);       
    }
    atom_printf("%s got\n", sem->name);
    
}

static void kmt_sem_signal(sem_t *sem){
    
    kmt_spin_lock(&sem->bin_lock);      
    sem->val++;       
    kmt_spin_unlock(&sem->bin_lock);       
    atom_printf("%s release\n", sem->name);
}

static void kmt_init(){
    kmt_spin_init(&task_list.lock, "task list lock");
    kmt_sem_init(&task_list.having, "task list sem" ,0);
    kmt_spin_init(&print_lock, "print lock");
}

static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){
    char buf[32];
    strncpy(buf,name,32);
    kmt->spin_init(&task->lock, strcat(buf, " spin lock") );
    task->status = RUNNABLE;
    task->entry = entry;
    task->arg = arg;
    task->context = kcontext((Area){.start=&task->fence + 1,.end=task+1},entry, arg);
    strncpy(task->name, name, KMT_NAME_SIZE);
    memset(task->fence, '6', KMT_FENCE_SIZE);
    return task_list_insert(task);

}

static void kmt_teardown(task_t *task){
    if(task->status==RUNNABLE || task->status==CAN_BE_CLEAR) task_list_delete(task);
    else{ // 正在RUNNING
        kmt->spin_lock(&task->lock);
        task->status=DEAD;// 当这个线程被调度或这个线程被时钟中断后，yield_handler会改变他的状态CAN_BE_CLEAR，并且调用tear_down
        kmt->spin_unlock(&task->lock);
    }
    
}

// 插入到task链表
int task_list_insert(task_t *task){
    kmt_spin_lock(&task_list.lock);
    panic_on(task==NULL,"task is NULL");
    if(task_list.head == NULL){
        task_list.head = task;
        task->next = task;
    }else{
        task->next = task_list.head->next;
        task_list.head->next = task;
    }

    kmt_spin_unlock(&task_list.lock);
    return 0;
    // V(&task_list.having);
}

// 从task链表中删除
void task_list_delete(task_t *task){
    kmt_spin_lock(&task_list.lock);
    panic_on(task==NULL,"task is NULL");
    if(task_list.head == NULL){
        panic("task list为空");
    }else{
        task_t *p = task_list.head;
        while(p->next != task){
            p = p->next;
        }
        // p--->task--->task->next
        p->next = task->next;
        if(task_list.head == task){
            task_list.head = p->next;
        }
    }
    pmm->free(task);
    kmt_spin_unlock(&task_list.lock);
    // P(&task_list.having);
}

void task_list_print(){
    kmt_spin_lock(&task_list.lock);
    task_t *p = task_list.head;
    while(p->next != task_list.head){
        atom_printf("task name is %s\n",p->name);
        p = p->next;
    }
    atom_printf("task name is %s\n",p->name);
    kmt_spin_unlock(&task_list.lock);
}


MODULE_DEF(kmt) = {
  	.init = kmt_init,
  	.create = kmt_create,
	.teardown = kmt_teardown,
	.sem_init = kmt_sem_init,
	.sem_wait = kmt_sem_wait,
	.sem_signal = kmt_sem_signal,
	.spin_init = kmt_spin_init,
	.spin_lock = kmt_spin_lock,
	.spin_unlock = kmt_spin_unlock

};
