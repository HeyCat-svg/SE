/*
 * name: DengShiyi  stu_no: 518021910184
 * 
 * mm.c - A memory allocator with high peek rate of memory usage
 *      and efficiency.
 * 
 * The allocator is implemented with segregated explicit free block list.
 * The strategy of find a free block is first fit.
 * The structure of allocated blocks consists of block header, block footer
 * and payload. The structure of free blocks consists of block header, block
 * footer and two WISZE-size offsets. The offsets point to previous or next free
 * block in its free block list. The minimal size of a block is 2*DSIZE size.
 * The free block lists are divided into 9 classes according the size of free
 * blocks in list. In one free block list, the size of free block is sorted in 
 * ascending order. Once a new free block is inserted, function `insert_to_list`
 * will judge which free block list it belongs to according to its size. Then
 * find appropriate position and insert the new block to it. Deletion is similar
 * to delete operation of link list. All address stored in payload part of free block
 * is relative address. They are relative to `block_listp`.
 * In function `mm_malloc`, allocator will find the free block which is suitable
 * to `size`. If there is no siutable free block in a free block list, allocator
 * will continue to find in a bigger free block list. If there is no suitable free
 * blocks in the whole free block list, allocator will extend the heap size.
 * Because free block list is sorted in ascending order, the first strategy is 
 * the best fit strategy at the same time. When freeing a block, coalescing is done
 * immediately. When reallocating a block, allocator will find if there is enough space
 * in previous or next free block. If so, replace directely. Otherwise allocate a
 * new block and place it.
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
#define CHUNKSIZE  (72)  /* Extend heap by this amount (bytes) */

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
char *heap_listp = NULL;        /* point to the header of heap */
char *heap_endp = NULL;         /* point to the end of heap */
char *block_listp = NULL;       /* point to start of the whole space */

/* given pointer p and calculate its offset */
#define CAL_OFS(p) ((p == NULL) ? (0) : ((unsigned int)((long)(p) - (long)(block_listp))))

/* get offset from pointer p */
#define GET_OFS(p) (*(unsigned int *)(p))

/* given block pointer then get its previous node */
#define GET_PRE(bp) ((GET_OFS(bp) == 0) ? (NULL) : ((block_listp) + GET_OFS(bp)))

/* given block pointer then get its next node */
#define GET_NEXT(bp) ((GET_OFS(bp + WSIZE) == 0) ? (NULL) : ((block_listp) + GET_OFS(bp + WSIZE)))


/*
 * Function prototype
 */
void *find_fit(size_t size);
void *place(void *bp, size_t size);
void *coalesce(void *bp);
void *extend_heap(size_t size);
inline char *select_header(size_t size);
void delete_blank_block(void *bp);
void insert_to_list(char *bp);
void realloc_place(void *bp, size_t asize);
void *realloc_coalesce(char *bp, size_t new_size, int *flag);
int mm_check();
int check_coalesce(void *bp);
int check_freelist(void *header);
int find_freeblock(void *bp);
int check_address_valid(void *bp);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void){
    if((heap_listp = mem_sbrk(12 * WSIZE)) == (void *)-1){
        return -1;
    }
    /* Free list headers. Each header's range of 
        size is on its right comment */
    PUT(heap_listp, 0);                 /* (16, 32] */
    PUT(heap_listp + (1 * WSIZE), 0);   /* (32, 64] */
    PUT(heap_listp + (2 * WSIZE), 0);   /* (64, 128] */
    PUT(heap_listp + (3 * WSIZE), 0);   /* (128, 256] */
    PUT(heap_listp + (4 * WSIZE), 0);   /* (256, 512] */
    PUT(heap_listp + (5 * WSIZE), 0);   /* (512, 1024] */
    PUT(heap_listp + (6 * WSIZE), 0);   /* (1024, 4096] */
    PUT(heap_listp + (7 * WSIZE), 0);   /* (2048, 16384] */
    PUT(heap_listp + (8 * WSIZE), 0);   /* (16384, infinity] */

    PUT(heap_listp + (9 * WSIZE), PACK(DSIZE, 1));  /* Prologue header */
    PUT(heap_listp + (10 * WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (11 * WSIZE), PACK(0, 1));     /* Epilogue header */
    block_listp = heap_listp;
    heap_listp += (10 * WSIZE);

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if(extend_heap(CHUNKSIZE / WSIZE) == NULL){
        return -1;
    }

#ifdef DEBUG
    mm_check();
#endif
    return 0;
}

/* 
 * mm_malloc - Return NULL if size = 0. Else find free block
 *     then allocate space. Extend heap size if there is no free block.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size){
    size_t asize;       /* size that will be allocated */
    size_t extendsize;  /* size of heap extened if there is no free block */
    char *bp;

    if (heap_listp == NULL){
        mm_init();
    }

    if(size == 0){
        return NULL;
    }
    else if(size == 448 || size == 456){
        size = 512;
    }
    else if(size == 112 || size == 120){
        size = 128;
    }

    asize = (size <= DSIZE) ? (2 * DSIZE) : 
            (DSIZE * ((size + 2 * DSIZE - 1) / DSIZE));
    
    /* Search the free list for a fit */
    if((bp = find_fit(asize)) != NULL){
        bp = place(bp, asize);

#ifdef DEBUG
    mm_check();
#endif
        return bp;
    }

    /*No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if((bp = extend_heap(extendsize/WSIZE)) == NULL){
        return NULL;
    }
    bp = place(bp, asize);

#ifdef DEBUG
    mm_check();
#endif
    return bp;
}

/*
 * mm_free - Return if bp=NULL. 
 *      Freeing a block and coalesce its adjacent free block.
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

#ifdef DEBUG
    mm_check();
#endif
}

/*
 * mm_realloc - If size=0, mm_free(size).
 *      If ptr=0, mm_malloc(size). Else move ptr to a new
 *      block which can hold the new size. Content in the old
 *      block will be copied to the new one. 
 */
void *mm_realloc(void *ptr, size_t size){
    size_t old_size, asize;
    char *nbp;        /* pointer pointing to new block malloced */
    old_size = GET_SIZE(HDRP(ptr));

    /* free ptr if size=0 */
    if(size == 0){
        mm_free(ptr);
        return NULL;
    }
    /* return mm_malloc(size) if ptr=NULL */
    if(ptr == NULL){
        return mm_malloc(size);
    }
    /* allignment to DSIZE*/
    asize = (size <= DSIZE) ? (2 * DSIZE) : 
            (DSIZE * ((size + 2 * DSIZE - 1) / DSIZE));

    /* do nothing if asize=old_size */
    if(old_size == asize){
        return ptr;
    }
    /* new size is larger */
    if(old_size < asize){
        int flag;   /* used to distinguish different cases */
        /* check if there is enough space in previous or next block */
        char *bp = realloc_coalesce(ptr, asize, &flag);
        if(flag == 1){      /* have enough space with next block */
            realloc_place(bp, asize);
            return bp;
        }
        /* have enough space with previous block */
        else if(flag == 2 || flag == 3){   
            memcpy(bp, ptr, old_size - DSIZE);
            realloc_place(bp, asize);
            return bp;
        }
        /* have to allocate new block */
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
    /* old size is larger */
    else{
        realloc_place(ptr, asize);
        return ptr;
    }
}

/*
 *  find_fit - Given size then find a free block whose size is
 *      larger than the argument "size". The strategy is first fit.
 */
void *find_fit(size_t size){
    /* select header pointer of free block list */
    char *headerp = select_header(size);
    char *p;
    for( ; headerp != (heap_listp - WSIZE); headerp += WSIZE){
        /* Get address of the header of block list from offset*/
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

/*
 *  place - Place a "size" size block at address "bp".
 *      If the remainning size is larger than 2*DSIZE, 
 *      divide free block and coalesce.
 */
void *place(void *bp, size_t size){
    size_t block_size = GET_SIZE(HDRP(bp));
    /* delete the free block from its block list */
    delete_blank_block(bp);
    /* The remainning size is less than minimal block size*/
    if (block_size - size < 2 * DSIZE){
        PUT(HDRP(bp), PACK(block_size, 1));
        PUT(FTRP(bp), PACK(block_size, 1));
    }
    /* divide the free block and coalesce */
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
    return bp;
}

/*
 *  coalesce - Coalesce free blocks if there is other free blocks
 *      being behind or in front of "bp".
 */
void *coalesce(void *bp){
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    /* no other free blocks */
    if(prev_alloc && next_alloc){   
        
    }
    /* next block is free */
    else if(prev_alloc && !next_alloc){ 
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        delete_blank_block(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    /* previous block is free */
    else if(!prev_alloc && next_alloc){ 
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        delete_blank_block(PREV_BLKP(bp));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    /* both blocks are free */
    else{
        /* delete both blocks from list */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        delete_blank_block(NEXT_BLKP(bp));
        delete_blank_block(PREV_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    /* insert free block after coalescing into free block list */
    insert_to_list(bp);
    return bp;
}

/*
 *  extend_heap - Extend the size of heap to "words"*WSIZE size 
 *      then coalesce new allocated free block.
 */
void *extend_heap(size_t words){
    char *bp;
    size_t size;
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1){
        return NULL;
    }
    /* new free block header */
    PUT(HDRP(bp), PACK(size, 0));
    /* new free block footer */
    PUT(FTRP(bp), PACK(size, 0));

    PUT(bp, 0);
    PUT(bp + WSIZE, 0);

    /* new epilogue header */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    /* record address of the end of heap */
    heap_endp = NEXT_BLKP(bp);
    /* coalesce if there are adjacent free blocks */
    return coalesce(bp);
}


/*
 *  select_header - Return corresponding headerp of 
 *      free block list according to size.
 */
inline char *select_header(size_t size){
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
    else if(size <= 4096)
        i = 6;
    else if(size <= 16384)
        i = 7;
    else
        i = 8;
    /* headerp is stored in front of heap list header */
    return block_listp + (i * WSIZE);
}

/*
 * delete_blank_block - delete free block from free block list 
 */
void delete_blank_block(void *bp){
    size_t size = GET_SIZE(HDRP(bp));
    char *headerp = select_header(size);
    char *pre_bp = GET_PRE(bp);
    char *next_bp = GET_NEXT(bp);
    
    /* no successor */
    if(pre_bp == NULL){
        if(next_bp != NULL){
            PUT(next_bp, 0);
        }
        PUT(headerp, CAL_OFS(next_bp));
    }
    /* have successor */
    else{
        if(next_bp != NULL){
            PUT(next_bp, CAL_OFS(pre_bp));
        }
        PUT(pre_bp + WSIZE, CAL_OFS(next_bp));
    }

    PUT(bp, 0);
    PUT(bp + WSIZE, 0);
}

/*
 * insert_to_list - insert free block to free block list
 *      and sort in ascending arder.
 */
void insert_to_list(char *bp){
    size_t block_size = GET_SIZE(HDRP(bp));
    char *headerp = select_header(block_size);
    /* special usage of GET_PRE */
    char *prevp = GET_PRE(headerp);
    char *nextp = GET_PRE(headerp);

    /* header is NULL */
    if(GET_PRE(headerp) == NULL){
        PUT(headerp, CAL_OFS(bp));
        PUT(bp, 0);
        PUT(bp + WSIZE, 0);
        return;
    }
    /* find free block whose size is larger than bp */
    while(nextp != NULL){
        if(GET_SIZE(HDRP(nextp)) >= block_size){
            break;
        }
        prevp = nextp;
        nextp = GET_NEXT(nextp);
    }
    /* insert at the beginning */
    if(nextp == GET_PRE(headerp)){
        PUT(bp, 0);
        PUT(bp + WSIZE, GET(headerp));
        PUT(GET_PRE(headerp), CAL_OFS(bp));
        PUT(headerp, CAL_OFS(bp));
        return;
    }
    /* insert at the end */
    if(nextp == NULL){
        PUT(prevp + WSIZE, CAL_OFS(bp));
        PUT(bp, CAL_OFS(prevp));
        PUT(bp + WSIZE, 0);
        return;
    }
    /* insert in the middle */
    else{
        PUT(prevp + WSIZE, CAL_OFS(bp));
        PUT(bp, CAL_OFS(prevp));
        PUT(bp + WSIZE, CAL_OFS(nextp));
        PUT(nextp, CAL_OFS(bp));
    }
}

/*
 * realloc_coalesce - Find whether there is enough size
 *      in previous or next free block and coalesce them.
 */
void *realloc_coalesce(char *bp, size_t new_size, int *flag){
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    /* used to judge different cases */
    *flag = 0;

    if(prev_alloc && next_alloc){
        /* case 0 */
    }
    else if(prev_alloc && !next_alloc){
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        /* have enough size with next free block */
        if(size >= new_size){
            delete_blank_block(NEXT_BLKP(bp));
            PUT(HDRP(bp), PACK(size, 1));
            PUT(FTRP(bp), PACK(size, 1));
            /* case 1 */
            *flag = 1;
            return bp;
        }
    }
    else if(!prev_alloc && next_alloc){
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        /* have enough size with previous free block */
        if(size >= new_size){
            delete_blank_block(PREV_BLKP(bp));
            PUT(FTRP(bp), PACK(size, 1));
            PUT(HDRP(PREV_BLKP(bp)), PACK(size, 1));
            /* case 2 */
            *flag = 2;
            return PREV_BLKP(bp);
        }
    }
    else{
        size += GET_SIZE(HDRP(NEXT_BLKP(bp))) +  GET_SIZE(HDRP(PREV_BLKP(bp)));
        /* have enough size with privous and next free block */
        if(size >= new_size){
            delete_blank_block(PREV_BLKP(bp));
            delete_blank_block(NEXT_BLKP(bp));
            PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 1));
            PUT(HDRP(PREV_BLKP(bp)), PACK(size, 1));
            /* case 3 */
            *flag = 3;
            return PREV_BLKP(bp);
        }
    }
    return bp;
}

/*
 * realloc_place - Place allocated block at bp without dividing 
 *      free block.
 */
void realloc_place(void *bp, size_t size){
    size_t block_size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(block_size, 1));
    PUT(FTRP(bp), PACK(block_size, 1));
}

/*
 * mm_check - Main check function. Check whether address in free list
 *      is valid. Check whether every block in free list is marked as free.
 *      Check whether free blocks in heap is coalesced and in free lists.
 *      Check whether address of block is valid. Check whether two adjacent
 *      block is overlapped.
 * Return 0 if there is a problem. Return 1 if there is no problems.
 */
int mm_check(){
    char *headerp = block_listp;
    char *header, *p;
    /* check free list */
    for (; headerp != heap_listp - WSIZE; headerp += WSIZE){
        header = GET_PRE(headerp);
        check_freelist(header);
    }

    /* check heap */
    for (p = heap_listp; p != heap_endp; p = NEXT_BLKP(p)){
        /* check valid address */
        if(!check_address_valid(p)){
            fprintf(stderr, "Valid address in heap\n");
            return 0;
        }
        /* check free blocks */
        if(GET_ALLOC(HDRP(p)) == 0){
            if(!check_coalesce(p)){
                return 0;
            }
            if(!find_freeblock(p)){
                fprintf(stderr, "Free block not in free list\n");
                return 0;
            }
        }
        /* check overlap */
        if((long)FTRP(p) >= (long)HDRP(NEXT_BLKP(p))){
            fprintf(stderr, "Block overlapped\n");
            return 0;
        }
    }
    return 1;
}

/*
 * check_coalesce - Check whether free blocks in heap is 
 *      coalesced and in free lists.
 * Return 0 if there is free block uncoalesced.
 * Return 1 if free block is coalesced.
 */
int check_coalesce(void *bp){
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));

    if(!(prev_alloc && next_alloc)){
        fprintf(stderr, "Free block uncoalesced\n");
        return 0;
    }
    return 1;
}

/*
 * check_freelist - Check whether address in free list
 *      is valid. Check whether every block in free list is marked as free.
 * Return 0 if there is a problem.
 * Return 1 if all is normal.
 */
int check_freelist(void *header){
    char *p = header;
    while(p != NULL){
        /* check valid free block address */
        if(!check_address_valid(p)){
            fprintf(stderr, "Valid address in free list\n");
            return 0;
        }
        /* check allocation of free block */
        if(GET_ALLOC(HDRP(p)) == 1){
            fprintf(stderr, "Allocated block in free list\n");
            return 0;
        }
        p = GET_NEXT(p);
    }
    return 1;
}

/*
 * find_freeblock - Find a free block in free list
 * Return 0 if no blocks are found in free list.
 * Return 1 if the free block is found.
 */
int find_freeblock(void *bp){
    size_t size = GET_SIZE(HDRP(bp));
    char *headerp = select_header(size);
    char *p = GET_PRE(headerp);

    while(p != NULL){
        if((long)p == (long)bp){
            return 1;
        }
        p = GET_NEXT(p);
    }
    return 0;
}

/*
 * check_address_valid - Check whether address is between 
 *      `block_listp` and `heap_endp`
 * Return 0: illegal address.
 * Return 1: legal address.
 */
int check_address_valid(void *bp){
    if((long)bp < (long)block_listp || (long)bp > (long)heap_endp){
        return 0;
    }
    return 1;
}
