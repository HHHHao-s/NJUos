#include <common.h>

int mm_init (void);
void *mm_malloc (size_t size);
void mm_free (void *ptr);

static int bin_lock;
static void *heap_top;


void lock(){while(atomic_xchg(&bin_lock, 1));};
void unlock(){atomic_xchg(&bin_lock, 0);};



static void *kalloc(size_t size) {
  lock();
  void *ret = mm_malloc(size);
  unlock();
  return ret;
}

static void kfree(void *ptr) {
  lock();
  mm_free(ptr);
  unlock();
}

static void pmm_init() {
  heap_top = heap.start;
  mm_init();
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};

void *mem_sbrk(size_t size){
  if(size + heap_top > heap.end){
    return NULL;
  }else{
    void *old_heap_top = heap_top;
    heap_top += size;
    return old_heap_top;
  }
}


#define ALIGNMENT 8

typedef void *Blockptr_t;
/* rounds up to the nearest multiple of ALIGNMENT */
#define PAGESIZE 4096
#define BIGBIT 0x80000000
#define NBP(bp, pos, i) ((Blockptr_t *)(bp) + ((i) * (1 << (pos)) / sizeof(Blockptr_t))) // the i of one block
#define NEXT(p) (*(Blockptr_t *)(p))
#define SETNEXT(p, np) ((*(Blockptr_t *)(p)) = (np))
#define exit(code) halt(code)

static inline char _log2(size_t size)
{ // return round up pos, pos is the position of bitvector
    int pos = 0;
    while (size > 1 << pos)
    {
        pos++;
    }
    return pos;
}

static unsigned int PAGEPOS;

Blockptr_t sizehead[32] = {NULL}; // from 8 to PAGESIZE/2 once init, last forever indentified by pos
Blockptr_t pagehead = NULL;       // list of unused page, once init, last forever
// Blockptr_t spanhead = NULL;

typedef struct _node
{
    void *start;
    unsigned int usedORbitpos_bigORlen;

} Page; // every page size is PAGESIZE

struct
{
    Page PageArray[1 << 25]; // i think that's enough to manage the whole memory
    size_t len;
} Manager;

void managererr(size_t index, void *bp)
{
    printf("getinfobybp len:%d index:%d bp:%p\n", Manager.len, index, bp);
}

Page getinfobybp(void *bp)
{ // get block information by bp
    size_t index = (bp - Manager.PageArray[0].start) / PAGESIZE;
    if (index > Manager.len)
    {
        managererr(index, bp);
    }

    return Manager.PageArray[index];
}

void setinfobybp(void *bp, Page info)
{
    size_t index = (bp - Manager.PageArray[0].start) / PAGESIZE;
    if (index > Manager.len)
    {
        managererr(index, bp);
    }

    Manager.PageArray[index] = info;
}

static inline int istop(void *bp)
{ // return 1 if the page of bp is one the top of heap
    return ((bp - Manager.PageArray[0].start) / PAGESIZE == Manager.len - 1);
}

void check(int pos)
{
    printf("checking %d\n", pos);
    for (Blockptr_t p = sizehead[pos]; p; p = NEXT(p))
    {
        printf("%p ", p);
    }
    printf("\n");
}

void checklist(){
    Blockptr_t p = pagehead;
    for(;p;p=NEXT(p)){
        printf("->%p", p);
    }
}

void *allocpage(unsigned int usedORbitpos_bigORlen)
{ // use sbrk and register this page

    void *p = mem_sbrk(PAGESIZE);
    if(p==NULL){
      return NULL;
    }
    Manager.PageArray[Manager.len++] = (Page){.start = p, .usedORbitpos_bigORlen = usedORbitpos_bigORlen};

    return p;
}

void *getpage(unsigned int usedORbitpos_bigORlen)
{ // get one page from page list for use

    if (pagehead == NULL)
    {

        return allocpage(usedORbitpos_bigORlen);
    }
    else
    {
        void *pret = pagehead;
        setinfobybp(pret, (Page){.start = pret, .usedORbitpos_bigORlen = usedORbitpos_bigORlen});
        pagehead = NEXT(pret);
        return pret; // return the second page of list
    }
    return NULL;
}

void *acquireonepage(int pos)
{ // acquire one initialized page

    unsigned int usedORbitpos_bigORlen = 1 | (1 << pos);

    Blockptr_t *p = getpage(usedORbitpos_bigORlen); // get one page for use
    if (pos < PAGEPOS)
    { // need to do sth.
        for (int i = 0; i < PAGESIZE / (1 << pos) - 1; i++)
        {
            SETNEXT(NBP(p, pos, i), NBP(p, pos, i + 1));
        }
        SETNEXT(NBP(p, pos, PAGESIZE / (1 << pos) - 1), NULL);
    }

    return p;
}

int mm_init(void)
{
    PAGEPOS = _log2(PAGESIZE);
    Manager.len = 0;
    Manager.PageArray[0] = (Page){.start = NULL, .usedORbitpos_bigORlen = 0};
    pagehead = NULL;

    for (int i = 3; i < PAGEPOS; i++)
    {
        sizehead[i] = acquireonepage(i);
    }

    return 0;
}

void *smallfind(int pos)
{ // from one list find one bp

    if (NEXT(sizehead[pos]) == NULL)
    { // only head
        Blockptr_t *p = acquireonepage(pos);
        SETNEXT(sizehead[pos], p);
    }

    Blockptr_t pret = NEXT(sizehead[pos]);
    if (pret != NULL)
    {
        SETNEXT(sizehead[pos], NEXT(pret));
    }
    else
    {
        exit(1);
    }

    return pret;
}


void *adjustpages(int pages,void *pto){
    for(int i=0;i<pages-1;i++){
        SETNEXT(pto, pto+PAGESIZE);
        setinfobybp(pto, (Page){.start = pto,.usedORbitpos_bigORlen=0});
        pto = pto+PAGESIZE;
    }
    SETNEXT(pto, NULL);
    setinfobybp(pto, (Page){.start = pto,.usedORbitpos_bigORlen=0});
    return pto;
}

void *bigfind(int pages)
{ // need how many pages

    Blockptr_t ret = NULL, pret = NULL; // pret is the previou of ret
    if (pagehead == NULL)
    {
        ret = allocpage(pages | (BIGBIT));
        for (int i = 1; i < pages; i++)
        {
            allocpage(BIGBIT | 1); // used and big, in free , it is an error
        }
    }
    else
    {
        int count = 1;
        Blockptr_t p = NEXT(pagehead), lp = pagehead, llp = NULL;
        ret = lp;

        while (p)
        { // find continuous pages
            if (lp + PAGESIZE == p) // continuous
            {
                if (count == 1)
                { // at the begining
                    count++;
                    pret = llp;
                    ret = lp; // maybe is the begining of pages
                }
                else
                {
                    count++;
                }
                if (pages == count)
                    break;
            }
            else
            { // not continuous
                ret = p;
                pret = lp;
                count = 1;
            }
            llp = lp;
            lp = p;
            p = NEXT(p);
        }

        if (count == pages)
        { // there is more place or is just pages
            if (ret == pagehead)
            {
                pagehead = NEXT(p); // p is NULL or other
            }
            else
            {
                SETNEXT(pret, NEXT(p)); // if ret != pagehead, then pret is not NULL
            }
        }
        else // p==NULL
        {
            
            if (istop(lp))
            {
                int last = pages - count;
                
                for (int i = 0; i < last; i++)
                {
                    allocpage(BIGBIT | 1);
                }
                
                if (ret == pagehead)
                {
                    pagehead = NULL;
                }
                else
                {
                    SETNEXT(pret, NULL);
                }
            }else{
                ret = allocpage(pages | (BIGBIT));
                
                for (int i = 0; i < pages; i++)
                {
                    allocpage(BIGBIT | 1); // used and big, in free , it is an error
                }
                if(pret != NULL)
                SETNEXT(pret, NULL);
            }
        }
    }
    if(ret != NULL)setinfobybp(ret, (Page){.start=ret,.usedORbitpos_bigORlen = (BIGBIT)|(pages)}) ;
    return ret;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int pos = _log2(size) > 3 ? _log2(size) : 3; // min is 3(_log2(8))
    void *ret = NULL;
    if (size > 1 << 24)
    {
        return NULL;
    }

    if (pos < PAGEPOS)
    {
        ret = smallfind(pos);
    }
    else if (pos == PAGEPOS)
    { // need one page
        ret = getpage(1 | (1 << PAGEPOS));
    }
    else
    {
        ret = bigfind((size + PAGESIZE - 1) / PAGESIZE);
    }

    return ret;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    Page info = getinfobybp(bp);

    unsigned int big = info.usedORbitpos_bigORlen & (BIGBIT); // bigbit
    if (!big)
    {
        unsigned int used = info.usedORbitpos_bigORlen & 1; // is used?

        if (used == 0)
        {
            printf("free error");
            return;
        } // there is no way to be there

        unsigned int posbit = info.usedORbitpos_bigORlen & ~1; // posbit

        int pos = _log2(posbit);
        if (pos < PAGEPOS)
        {
            Blockptr_t p = sizehead[pos];
            Blockptr_t lp = NULL;
            for (; p && bp > p; p = NEXT(p))
            { // 找到正确的位置，可能是中间，也可能是最后
                lp = p;
            }
            if (p && bp < p)
            {
                SETNEXT(lp, bp);
                SETNEXT(bp, p);
            }
            else if (!p)
            { // end of list
                SETNEXT(lp, bp);
                SETNEXT(bp, NULL);
            }
        }
        else
        {
            Blockptr_t p = pagehead;
            Blockptr_t lp = NULL;
            if (p == NULL)
            {
                pagehead = bp;
                SETNEXT(pagehead, NULL);
            }
            else
            {
                if (p < bp)
                {
                    for (; p && bp > p; p = NEXT(p))
                    { // 找到正确的位置，可能是中间，也可能是最后
                        lp = p;
                    }

                    if (p && bp < p)
                    {
                        SETNEXT(lp, bp);
                        SETNEXT(bp, p);
                    }
                    else if (!p)
                    { // end of list
                        SETNEXT(lp, bp);
                        SETNEXT(bp, NULL);
                    }
                }
                else
                { // bp < p

                    SETNEXT(bp, pagehead);
                    pagehead = bp;
                }
            }
            setinfobybp(bp, (Page){.start = bp, .usedORbitpos_bigORlen = 0});
        }
    }
    else
    {
        int pages = info.usedORbitpos_bigORlen & (~(BIGBIT));
        void *pend = adjustpages(pages, bp); 
        
    
        if (pagehead == NULL)
        {
            pagehead = bp;
      
            SETNEXT(pend,NULL);
                        
        }
        else
        {   
            
            if (pagehead < bp)
            {
                Blockptr_t p = pagehead;
                Blockptr_t lp = NULL;
                for (; p && bp > p; p = NEXT(p))
                { // 找到正确的位置，可能是中间，也可能是最后
                    lp = p;
                }
                // bp is between lp and p

                SETNEXT(lp,bp);
               
                SETNEXT(pend,p);
                               
            }
            else
            { // bp < p

                SETNEXT(pend, pagehead);
                pagehead = bp;

            }
        }
        
    }
    // checklist();
}

void *mm_realloc(void *ptr, size_t size)
{
    return NULL;
}