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


/*
 * Global variables
 */

char *heap_listp = NULL;
char *block_listp = NULL;

/* Other macros */
#define CAL_OFS(p) ((p == NULL) ? (0) : ((unsigned int)((long)(p) - (long)(block_listp))))
#define GET_OFS(p) (*(unsigned int *)(p))
#define GET_PRE(bp) ((GET_OFS(bp) == 0) ? (NULL) : ((block_listp) + GET_OFS(bp)))
#define GET_NEXT(bp) ((GET_OFS(bp + WSIZE) == 0) ? (NULL) : ((block_listp) + GET_OFS(bp + WSIZE)))


/*
 * Function prototype
 */
void *find_fit(size_t size);
void place(void *bp, size_t size);
void *coalesce(void *bp);
void *extend_heap(size_t size);
char *select_header(size_t size);
void delete_blank_block(void *bp);
void insert_to_list(char *bp);

void realloc_place(void *bp, size_t asize);
void *realloc_coalesce(char *bp, size_t new_size, int *flag);


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void){
    if((heap_listp = mem_sbrk(12 * WSIZE)) == (void *)-1){
        return -1;
    }
    PUT(heap_listp, 0);                 /* (16, 32] */
    PUT(heap_listp + (1 * WSIZE), 0);   /* (32, 64] */
    PUT(heap_listp + (2 * WSIZE), 0);   /* (64, 128] */
    PUT(heap_listp + (3 * WSIZE), 0);   /* (128, 256] */
    PUT(heap_listp + (4 * WSIZE), 0);   /* (256, 512] */
    PUT(heap_listp + (5 * WSIZE), 0);   /* (512, 1024] */
    PUT(heap_listp + (6 * WSIZE), 0);   /* (1024, 2048] */
    PUT(heap_listp + (7 * WSIZE), 0);   /* (2048, 4096] */
    PUT(heap_listp + (8 * WSIZE), 0);   /* (4096, infinity] */

    PUT(heap_listp + (9 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (10 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (11 * WSIZE), PACK(0, 1));

    block_listp = heap_listp;
    heap_listp += (10 * WSIZE);
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

    if (heap_listp == NULL){
        mm_init();
    }

    if(size == 0){
        return NULL;
    }

    asize = (size <= DSIZE) ? (2 * DSIZE) : 
            (DSIZE * ((size + 2 * DSIZE - 1) / DSIZE));
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
    if (heap_listp == 0){
        mm_init();
    }

    size_t size = GET_SIZE(HDRP(bp));
    
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(bp, 0);
    PUT(bp + WSIZE, 0);
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size){
    size_t old_size, asize;
    char *nbp;
    old_size = GET_SIZE(HDRP(ptr));

    if(size == 0){
        mm_free(ptr);
        return NULL;
    }
    if(ptr == NULL){
        return mm_malloc(size);
    }

    if(size <= DSIZE){
        asize = 2 * DSIZE;
    }
    else{
        asize = DSIZE * ((size + 2 * DSIZE - 1) / DSIZE);
    }

    if(old_size == asize){
        return ptr;
    }

    // {
    //     // test
    //     nbp = mm_malloc(asize);
    //     if(nbp == NULL){
    //         return NULL;
    //     }
    //     memcpy(nbp, ptr, old_size - DSIZE);
    //     mm_free(ptr);
    //     return nbp;
    // }

    if(old_size < asize){
        int flag;
        char *bp = realloc_coalesce(ptr, asize, &flag);
        if(flag == 1){
            realloc_place(bp, asize);
            return bp;
        }
        else if(flag == 2 || flag == 3){
            memcpy(bp, ptr, old_size - DSIZE);
            realloc_place(bp, asize);
            return bp;
        }
        else{
            nbp = mm_malloc(asize);
            if(nbp == NULL){
                return NULL;
            }
            memcpy(nbp, ptr, old_size - DSIZE);
            mm_free(ptr);
            return nbp;
        }
    }
    else{
        realloc_place(ptr, asize);
        return ptr;
    }
}

void *find_fit(size_t size){
    /* First fit */
    char *headerp = select_header(size);
    char *p;
    for( ; headerp != (heap_listp - WSIZE); headerp += WSIZE){
        p = GET_PRE(headerp);
        while(p != NULL){
            if (GET_SIZE(HDRP(p)) >= size){
                return p;
            }
            p = GET_NEXT(p);
        }
    }
    return NULL;
}

void place(void *bp, size_t size){
    size_t block_size = GET_SIZE(HDRP(bp));
    delete_blank_block(bp);
    if (block_size - size < 2 * DSIZE){
        PUT(HDRP(bp), PACK(block_size, 1));
        PUT(FTRP(bp), PACK(block_size, 1));
    }
    else{
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
        char *curBp = NEXT_BLKP(bp);
        PUT(HDRP(curBp), PACK(block_size - size, 0));
        PUT(FTRP(curBp), PACK(block_size - size, 0));

        PUT(curBp, 0);
        PUT(curBp + WSIZE, 0);
        coalesce(curBp);
    }
}

void *coalesce(void *bp){
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc){
        
    }
    else if(prev_alloc && !next_alloc){
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        delete_blank_block(NEXT_BLKP(bp));

        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if(!prev_alloc && next_alloc){
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        delete_blank_block(PREV_BLKP(bp));

        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));

        bp = PREV_BLKP(bp);
    }
    else{
        /* delete third block from list */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        delete_blank_block(NEXT_BLKP(bp));
        delete_blank_block(PREV_BLKP(bp));

        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
            
        bp = PREV_BLKP(bp);
    }
    insert_to_list(bp);
    return bp;
}

void *extend_heap(size_t words){
    char *bp;
    size_t size;
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1){
        return NULL;
    }
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    PUT(bp, 0);
    PUT(bp + WSIZE, 0);

    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    return coalesce(bp);
}

char *select_header(size_t size){
    int i = 0;
    if(size <= 32)
        i = 0;
    else if(size <= 64)
        i = 1;
    else if(size <= 128)
        i = 2;
    else if(size <= 256)
        i = 3;
    else if(size <= 512)
        i = 4;
    else if(size <= 1024)
        i = 5;
    else if(size <= 2048)
        i = 6;
    else if(size <= 4096)
        i = 7;
    else
        i = 8;
    return block_listp + (i * WSIZE);
}

void delete_blank_block(void *bp){
    size_t size = GET_SIZE(HDRP(bp));
    char *headerp = select_header(size);
    char *pre_bp = GET_PRE(bp);
    char *next_bp = GET_NEXT(bp);
    
    if(pre_bp == NULL){
        if(next_bp != NULL){
            PUT(next_bp, 0);
        }
        PUT(headerp, CAL_OFS(next_bp));
    }
    else{
        if(next_bp != NULL){
            PUT(next_bp, CAL_OFS(pre_bp));
        }
        PUT(pre_bp + WSIZE, CAL_OFS(next_bp));
    }

    PUT(bp, 0);
    PUT(bp + WSIZE, 0);
}

void insert_to_list(char *bp){
    size_t block_size = GET_SIZE(HDRP(bp));
    char *headerp = select_header(block_size);
    char *prevp = GET_PRE(headerp);
    char *nextp = GET_PRE(headerp);

    if(GET_PRE(headerp) == NULL){   // special usage of GET_PRE
        PUT(headerp, CAL_OFS(bp));
        PUT(bp, 0);
        PUT(bp + WSIZE, 0);
        return;
    }
    while(nextp != NULL){
        if(GET_SIZE(HDRP(nextp)) >= block_size){
            break;
        }
        prevp = nextp;
        nextp = GET_NEXT(nextp);
    }
    if(nextp == GET_PRE(headerp)){  // special usage of GET_PRE
        PUT(bp, 0);
        PUT(bp + WSIZE, GET(headerp));
        PUT(GET_PRE(headerp), CAL_OFS(bp));
        PUT(headerp, CAL_OFS(bp));
        return;
    }
    if(nextp == NULL){
        PUT(prevp + WSIZE, CAL_OFS(bp));
        PUT(bp, CAL_OFS(prevp));
        PUT(bp + WSIZE, 0);
        return;
    }
    else{
        PUT(prevp + WSIZE, CAL_OFS(bp));
        PUT(bp, CAL_OFS(prevp));
        PUT(bp + WSIZE, CAL_OFS(nextp));
        PUT(nextp, CAL_OFS(bp));
    }
}


void *realloc_coalesce(char *bp, size_t new_size, int *flag){
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    *flag = 0;

    if(prev_alloc && next_alloc){
        
    }
    else if(prev_alloc && !next_alloc){
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        if(size >= new_size){
            delete_blank_block(NEXT_BLKP(bp));
            PUT(HDRP(bp), PACK(size, 1));
            PUT(FTRP(bp), PACK(size, 1));
            *flag = 1;
            return bp;
        }
    }
    else if(!prev_alloc && next_alloc){
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        if(size >= new_size){
            delete_blank_block(PREV_BLKP(bp));
            PUT(FTRP(bp), PACK(size, 1));
            PUT(HDRP(PREV_BLKP(bp)), PACK(size, 1));
            *flag = 2;
            return PREV_BLKP(bp);
        }
    }
    else{
        size += GET_SIZE(HDRP(NEXT_BLKP(bp))) +  GET_SIZE(HDRP(PREV_BLKP(bp)));
        if(size > new_size){
            delete_blank_block(PREV_BLKP(bp));
            delete_blank_block(NEXT_BLKP(bp));
            PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 1));
            PUT(HDRP(PREV_BLKP(bp)), PACK(size, 1));
            *flag = 3;
            return PREV_BLKP(bp);
        }
    }
    return bp;
}

void realloc_place(void *bp, size_t size){
    size_t block_size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(block_size, 1));
    PUT(FTRP(bp), PACK(block_size, 1));
}
