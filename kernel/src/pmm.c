#include <common.h>

#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (sizeof(size_t))

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(size_t *)(p))
#define PUT(p, val) (*(size_t *)(p) = (size_t)(val))

#define PUT_PTR(p, ptr) (*(void **)(p) = (void *)(ptr))

#define GET_SIZE(p) ((*(size_t *)(p)) & (size_t)~0x7)
#define GET_ALLOC(p) ((*(size_t *)(p)) & 0x1)
#define HDRP(bp) ((char *)(bp)-SIZE_T_SIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - 2 * SIZE_T_SIZE)

// given bp, get next or previous bp
#define NEXT_BLK(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-SIZE_T_SIZE)))
#define PREV_BLK(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-2 * SIZE_T_SIZE)))

#define SUCC_ADD(bp) (((void **)(bp)) + 1) // the successor pointer
#define PRED_ADD(bp) (((void **)(bp)))     // the predecessor pointer

#define SUCC_BLK(bp) (*((void **)(SUCC_ADD(bp)))) // the successor block address
#define PRED_BLK(bp) (*((void **)(PRED_ADD(bp)))) // the predecessor block address

static void *head = NULL; // the head of free list

static int bin_lock;

static void lock()
{
    while (atomic_xchg(&bin_lock, 1))
        ;
}

static void unlock()
{
    atomic_xchg(&bin_lock, 0);
}

static void *place(size_t size)
{

    void *bp = head;
    while (GET_ALLOC(HDRP(bp)) == 1 || GET_SIZE(HDRP(bp)) < size)
    {

        bp = SUCC_BLK(bp);
        if (bp == NULL)
        {
            return NULL;
        }
    }
    // get an unalloc address
    size_t csize = GET_SIZE(HDRP(bp)) - size;

    if (csize < 4 * SIZE_T_SIZE)
    {
        PUT(HDRP(bp), PACK(GET_SIZE(HDRP(bp)), 1));
        PUT(FTRP(bp), PACK(GET_SIZE(HDRP(bp)), 1));

        void *pred_bp = PRED_BLK(bp);
        void *succ_bp = SUCC_BLK(bp);

        if (succ_bp != NULL)
            PUT_PTR(PRED_ADD(succ_bp), pred_bp);
        if (pred_bp != NULL)
            PUT_PTR(SUCC_ADD(pred_bp), succ_bp);
    }
    else
    {
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));

        void *pred_bp = PRED_BLK(bp);
        void *succ_bp = SUCC_BLK(bp);

        void *next_bp = NEXT_BLK(bp);

        PUT(HDRP(next_bp), PACK(csize, 0));
        PUT(FTRP(next_bp), PACK(csize, 0));

        PUT_PTR(PRED_ADD(next_bp), pred_bp);
        PUT_PTR(SUCC_ADD(next_bp), succ_bp);

        if (succ_bp != NULL)
            PUT_PTR(PRED_ADD(succ_bp), next_bp);
        if (pred_bp != NULL)
            PUT_PTR(SUCC_ADD(pred_bp), next_bp);
    }

    return bp;
}

static void coalesce(void *bp)
{
    char prealloc, nextalloc;
    void *prebp = PREV_BLK(bp), *nextbp = NEXT_BLK(bp);
    prealloc = GET_ALLOC(HDRP(prebp));
    nextalloc = GET_ALLOC(HDRP(nextbp));

    size_t cursize = GET_SIZE(HDRP(bp));

    if (prealloc == 1 && nextalloc == 1)
    {

        if ((size_t)head > (size_t)bp)
        {

            PUT_PTR(SUCC_ADD(bp), head);
            PUT_PTR(PRED_ADD(bp), NULL);

            PUT_PTR(PRED_ADD(bp), bp);
            head = bp;
        }
        else
        {
            void *p1 = NULL, *p2 = head; // for use
            p2 = head;

            while (p2 < bp && p2 != NULL)
            { // find two blocks that bp between which
                p1 = p2;
                p2 = SUCC_BLK(p2);
            }

            PUT_PTR(SUCC_ADD(p1), bp);

            if (p2 != NULL)
                PUT_PTR(PRED_ADD(p2), bp);

            PUT_PTR(PRED_ADD(bp), p1);
            PUT_PTR(SUCC_ADD(bp), p2);
        }
    }
    else if (prealloc == 0 && nextalloc == 1)
    {
        size_t size = cursize + GET_SIZE(HDRP(prebp));
        PUT(HDRP(prebp), PACK(size, 0));
        PUT(FTRP(prebp), PACK(size, 0));
    }
    else if (prealloc == 1 && nextalloc == 0)
    {
        size_t size = cursize + GET_SIZE(HDRP(nextbp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));

        PUT_PTR(PRED_ADD(bp), PRED_BLK(nextbp));
        PUT_PTR(SUCC_ADD(bp), SUCC_BLK(nextbp));

        if (SUCC_BLK(nextbp) != NULL)
        {
            PUT_PTR(PRED_ADD(SUCC_BLK(nextbp)), bp);
        }
        if (PRED_BLK(nextbp) != NULL)
        {
            PUT_PTR(SUCC_ADD(PRED_BLK(nextbp)), bp);
        }

        if (head == nextbp)
            head = bp;
    }
    else
    {
        size_t size = cursize + GET_SIZE(HDRP(prebp)) + GET_SIZE(HDRP(nextbp));
        PUT(HDRP(prebp), PACK(size, 0));
        PUT(FTRP(prebp), PACK(size, 0));

        PUT_PTR(SUCC_ADD(prebp), SUCC_BLK(nextbp));
        if (SUCC_BLK(nextbp) != NULL)
            PUT_PTR(SUCC_ADD(SUCC_BLK(nextbp)), prebp);
    }
    return;
}

static void *kalloc(size_t size)
{
    lock();
    size_t newsize = ALIGN(size + 4 * SIZE_T_SIZE); // DSIZE is the minimum size of a block
    void *ret = place(newsize);
    unlock();
    return ret;
}

static void kfree(void *bp)
{

    if (GET_ALLOC(HDRP(bp)) == 1)
    {   lock();
        PUT(HDRP(bp), PACK(GET_SIZE(HDRP(bp)), 0));
        PUT(FTRP(bp), PACK(GET_SIZE(HDRP(bp)), 0)); // alloc then free
        PUT(SUCC_ADD(bp), NULL);
        PUT(PRED_ADD(bp), NULL);
        coalesce(bp);
        unlock();
    }
}

static void pmm_init()
{
    void *plo = heap.start;

    PUT(plo + 1 * SIZE_T_SIZE, PACK(4 * SIZE_T_SIZE, 1));
    PUT(plo + 4 * SIZE_T_SIZE, PACK(4 * SIZE_T_SIZE, 1));
    PUT_PTR(plo + 2 * SIZE_T_SIZE, NULL);
    PUT_PTR(plo + 3 * SIZE_T_SIZE, NULL);

    PUT(plo + 5 * SIZE_T_SIZE, PACK(0, 1));

    head = plo + 2 * SIZE_T_SIZE;

}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
