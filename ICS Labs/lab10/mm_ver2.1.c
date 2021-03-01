/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/* $begin mallocmacros */
/* Basic constants and macros */
#define WSIZE       4       /* Word and header/footer size (bytes) */
#define DSIZE       8       /* Double word size (bytes) */
#define CHUNKSIZE  (1 << 12)  /* Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y)? (x) : (y))  

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Other macros */
#define MINUS(a, b) ((int)(((long)(a)) - ((long)(b))))
#define GET_OFS(p) (*(int *)(p))
#define GET_PRE(bp) ((GET_OFS(bp) == 0) ? (0) : (bp + GET_OFS(bp)))
#define GET_NEXT(bp) ((GET_OFS(bp + WSIZE) == 0) ? (0) : (bp + GET_OFS(bp + WSIZE)))



/*
 * Function prototype
 */
void *find_fit(size_t size);
void place(void *bp, size_t size);
void *coalesce(void *bp);
void *extend_heap(size_t size);
void delete_blank_block(void *bp);
void add_to_header(void *bp);

/*
 * Global variables
 */

static char *heap_listp = NULL;
static char *header = NULL;

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void){
    if((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1){
        return -1;
    }
    PUT(heap_listp, 0);
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));
    heap_listp += (2 * WSIZE);
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL){
        return -1;
    }
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size){
    size_t asize;
    size_t extendsize;
    char *bp;

    if (heap_listp == 0){
        mm_init();
    }

    if(size == 0){
        return NULL;
    }

    if(size <= DSIZE){
        asize = 2 * DSIZE;
    }
    else{
        asize = DSIZE * ((size + 2 * DSIZE - 1) / DSIZE);
    }
    if((bp = find_fit(asize)) != NULL){
        place(bp, asize);
        return bp;
    }

    extendsize = MAX(asize, CHUNKSIZE);
    if((bp = extend_heap(extendsize/WSIZE)) == NULL){
        return NULL;
    }
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp){
    if(bp == 0){
        return;
    }
    size_t size = GET_SIZE(HDRP(bp));

    if (heap_listp == 0){
        mm_init();
    }

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size){
    size_t block_size, asize;
    long minus_size;
    char *nbp;

    if(ptr == NULL){
        return mm_malloc(size);
    }
    if(size == 0){
        mm_free(ptr);
        return NULL;
    }

    block_size = GET_SIZE(HDRP(ptr));
    asize = (size <= DSIZE) ? (2 * DSIZE) : (DSIZE * ((size + 2 * DSIZE - 1) / DSIZE));
    minus_size = (long)block_size - (long)asize;
    if (minus_size > 0 && minus_size < 2 * DSIZE){
        return ptr;
    }
    else if(minus_size > 0){
        PUT(HDRP(ptr), PACK(asize, 1));
        PUT(FTRP(ptr), PACK(asize, 1));
        char *curBp = NEXT_BLKP(ptr);
        PUT(HDRP(curBp), PACK((size_t)minus_size, 0));
        PUT(FTRP(curBp), PACK((size_t)minus_size, 0));

        coalesce(curBp);
        return ptr;
    }
    else if(minus_size < 0){
        nbp = mm_malloc(asize);
        if(nbp == NULL){
            return NULL;
        }
        memcpy(nbp, ptr, block_size - DSIZE);
        mm_free(ptr);
        return nbp;
    }
    return ptr;
}

void *find_fit(size_t size){
    /* First fit */
    char *p = header;
    while(p != NULL){
        if (GET_SIZE(HDRP(p)) >= size){
            return p;
        }
        p = GET_NEXT(p);
    }
    return NULL;
}

void place(void *bp, size_t size){
    size_t block_size = GET_SIZE(HDRP(bp));
    if (block_size - size < 2 * DSIZE){
        PUT(HDRP(bp), PACK(block_size, 1));
        PUT(FTRP(bp), PACK(block_size, 1));

        /* Delete this block from list */
        delete_blank_block(bp);
    }
    else{
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
        char *curBp = NEXT_BLKP(bp);
        PUT(HDRP(curBp), PACK(block_size - size, 0));
        PUT(FTRP(curBp), PACK(block_size - size, 0));

        char *preBp = GET_PRE(bp);
        char *nextBp = GET_NEXT(bp);
        if(preBp != NULL){
            PUT(curBp, GET_OFS(bp) - size);
            PUT(preBp + WSIZE, GET_OFS(preBp + WSIZE) + size);
        }
        else{
            PUT(curBp, 0);
            header = curBp;
        }
        if(nextBp != NULL){
            PUT(curBp + WSIZE, GET_OFS(bp + WSIZE) - size);
            PUT(nextBp, GET_OFS(nextBp) + size);
        }
        else{
            PUT(curBp + WSIZE, 0);
        }
    }
}

void *coalesce(void *bp){
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc){
        /* add to header */
        add_to_header(bp);
        return bp;
    }
    else if(prev_alloc && !next_alloc){
        char *curBp = NEXT_BLKP(bp);
        char *preBp =GET_PRE(curBp);
        char *nextBp = GET_NEXT(curBp);
        if(preBp != 0){
            PUT(bp, GET_OFS(curBp) + size);
            PUT(preBp + WSIZE, GET_OFS(preBp + WSIZE) - size);
        }
        else{
            PUT(bp, 0);
            header = bp;
        }
        if(nextBp != 0){
            PUT(bp + WSIZE, GET_OFS(curBp + WSIZE) + size);
            PUT(nextBp, GET_OFS(nextBp) - size);
        }
        else{
            PUT(bp + WSIZE, 0);
        }

        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if(!prev_alloc && next_alloc){
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else{
        /* delete third block from list */
        char *curBp = NEXT_BLKP(bp);
        delete_blank_block(curBp);

        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}

void *extend_heap(size_t words){
    char *bp;
    size_t size;
    size = (words%2) ? (words+1)*WSIZE : words*WSIZE;
    if((long)(bp = mem_sbrk(size)) == -1){
        return NULL;
    }
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);
}

void delete_blank_block(void *bp){
    char *pre_bp = GET_PRE(bp);
    char *next_bp = GET_NEXT(bp);
    
    if (bp == header){
        header = next_bp;
        if(header != NULL){
            PUT(header, 0);
        }
    }
    else if(next_bp == NULL){
        PUT(pre_bp + WSIZE, 0);
    }
    else{
        PUT(pre_bp + WSIZE, MINUS(next_bp, pre_bp));
        PUT(next_bp, MINUS(pre_bp, next_bp));
    }
}

void add_to_header(void *bp){
    if(header == NULL){
        header = bp;
        PUT(header, 0);
        PUT(header + WSIZE, 0);
    }
    else{
        PUT(header, MINUS(bp, header));
        PUT(bp + WSIZE, MINUS(header, bp));
        PUT(bp, 0);
        header = bp;
    }
}