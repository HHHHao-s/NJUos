#include <os.h>


void kmt_init(){
    
}

void kmt_spin_init(spinlock_t *lk, const char *name){
    lk->lock = 1;
    lk->name = name;
}

void kmt_spin_lock(spinlock_t *lk){
    while(!atomic_xchg(&lk->lock,0));
}

void kmt_spin_unlock(spinlock_t *lk){
    panic_on(lk->lock!=0,"释放的自旋锁不是0");
    atomic_xchg(&lk->lock, 1);
}

void kmt_sem_init(sem_t *sem, const char *name, int value){
    kmt_spin_init(&sem->bin_lock,"信号量的自旋锁" );
    sem->name = name;
    sem->val = value;
}


void kmt_sem_wait(sem_t *sem){
    while(1){
        kmt_spin_lock(&sem->bin_lock);
        if(sem->val>0){
            sem->val--;
            kmt_spin_unlock(&sem->bin_lock);
            break;
        }
        kmt_spin_unlock(&sem->bin_lock);       
    }
    
}

void kmt_sem_signal(sem_t *sem){
    
    kmt_spin_lock(&sem->bin_lock);      
    sem->val++;       
    kmt_spin_unlock(&sem->bin_lock);       
    
}

int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){
    return 0;
}
void kmt_teardown(task_t *task){
	;
}

void (*init)();
int  (*create)(task_t *task, const char *name, void (*entry)(void *arg), void *arg);
void (*teardown)(task_t *task);
void (*spin_init)(spinlock_t *lk, const char *name);
void (*spin_lock)(spinlock_t *lk);
void (*spin_unlock)(spinlock_t *lk);
void (*sem_init)(sem_t *sem, const char *name, int value);
void (*sem_wait)(sem_t *sem);
void (*sem_signal)(sem_t *sem);











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
