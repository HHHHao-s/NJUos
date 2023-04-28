#include <hashmap.h>

void hash_add(void *_pa, int _ref){
    Map_node_t *adder = (Map_node_t *)pmm->alloc(sizeof(Map_node_t));
    *adder = (Map_node_t){.pa=(uintptr_t)_pa, .ref=_ref, .next = NULL};

    int base = (((uintptr_t)_pa)>>12)%MAP_SIZE;

    if(head[base]==NULL){
        head[base] = adder;
    }else{
        Map_node_t *tmp = head[base];
        while(tmp->next!=NULL){
            tmp = tmp->next;
        }
        tmp->next = adder;
    }
}




// 找到返回ref，找不到返回0
int hash_find(void *pa){
    int base = (((uintptr_t)pa)>>12)%MAP_SIZE;
    Map_node_t *tmp = head[base];
    while(tmp!=NULL){
        if(tmp->pa == (uintptr_t)pa){
            return tmp->ref;
        }
        tmp = tmp->next;
    }
    return 0;
}


void hash_delete(void *pa){
    int base = (((uintptr_t)pa)>>12)%MAP_SIZE;
    Map_node_t *tmp = head[base];
    Map_node_t *pre = NULL;
    while(tmp!=NULL){
        if(tmp->pa == (uintptr_t)pa){
            if(pre==NULL){
                head[base] = tmp->next;
            }else{
                pre->next = tmp->next;
            }
            pmm->free(tmp);
            return;
        }
        pre = tmp;
        tmp = tmp->next;
    }
    
}


void increase(void *pa){
    int base = (((uintptr_t)pa)>>12)%MAP_SIZE;
    Map_node_t *tmp = head[base];
    while(tmp!=NULL){
        if(tmp->pa == (uintptr_t)pa){
            tmp->ref++;
            return;
        }
        tmp = tmp->next;
    }
    hash_add(pa, 1); // 到这里为空，添加进去
}

void decrease(void *pa){
    int base = (((uintptr_t)pa)>>12)%MAP_SIZE;
    Map_node_t *tmp = head[base];
    while(tmp!=NULL){
        if(tmp->pa == (uintptr_t)pa){
            tmp->ref--;
            if(tmp->ref==0){
                hash_delete(pa);
            }
            return;
        }
        tmp = tmp->next;
    }
}