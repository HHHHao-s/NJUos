

#define MAP_SIZE 64


typedef struct map_node_t {
    uintptr_t pa;
    int ref;
    struct map_node_t *next;
}Map_node_t;

Map_node_t *head[MAP_SIZE];



void hash_add(void *_pa, int _ref);




// 找到返回ref，找不到返回0
int hash_find(void *pa);


void hash_delete(void *pa);


void increase(void *pa);

void decrease(void *pa);
