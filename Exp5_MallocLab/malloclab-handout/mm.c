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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    "USTC-Z", /* Team name */

    "Yinnan Zhou",                 /* First member full name */
    "yinnanzhou@mail.ustc.edu.cn", /* First member email address */

    "", /* Second member full name (leave blank if none) */
    ""  /* Second member email addr (leave blank if none) */
};
/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

// Macro definition
#define WSIZE 4             /* Word and header/footer size (bytes) */
#define DSIZE 8             /* Doubleword size (bytes) */
#define CHUNKSIZE (1 << 12) /* Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int*)(p))
#define PUT(p, val) (*(unsigned int*)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char*)(bp)-WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(((char*)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char*)(bp)-GET_SIZE(((char*)(bp)-DSIZE)))

static char* heap_listp;
static void* extend_heap(size_t words);
static void* coalesce(void* bp);
static void place(void* bp, size_t asize);
static void* first_fit(size_t asize);
static void* best_fit(size_t asize);

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
    // Allocate initial heap space and check for errors
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void*)-1)
        return -1;

    // Set up the initial empty heap with a prologue and epilogue
    PUT(heap_listp, 0);                             // Alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));  // Prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));  // Prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));      // Epilogue header
    heap_listp += (2 * WSIZE);  // Move heap pointer past prologue

    // Extend the heap with an initial free block
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;

    // Return 0 to indicate successful initialization
    return 0;
}

/*
 * extend_heap - Extend the heap by a specified number of words.
 */
static void* extend_heap(size_t words) {
    char* bp;
    size_t size;

    // Calculate the size to allocate, ensuring it is aligned
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;

    // Attempt to allocate the requested size from the heap
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    // Initialize the header and footer of the newly allocated block
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    // Create the epilogue header to indicate the end of the heap
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    // Coalesce the newly allocated block with any adjacent free blocks
    return coalesce(bp);
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void* mm_malloc(size_t size) {
    size_t asize;
    size_t extendsize;
    char* bp;

    // Ensure that requested size is not zero
    if (size == 0) {
        return NULL;
    }

    // Adjust block size to include overhead and alignment requirements
    if (size <= DSIZE) {
        asize = 2 * DSIZE;
    } else {
        asize = DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE);
    }

    // Try to find a suitable free block using best fit strategy
    if ((bp = best_fit(asize)) != NULL) {
    // if ((bp = first_fit(asize)) != NULL) {
        // Allocate and place the block
        place(bp, asize);
        return bp;
    }

    // If no suitable block is found, extend the heap
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL) {
        return NULL;
    }

    // Allocate and place the block in the newly extended heap
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void* bp) {
    // Get the size of the block
    size_t size = GET_SIZE(HDRP(bp));

    // Update the header and footer to mark the block as unallocated
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    // Coalesce the freed block with adjacent free blocks
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void* mm_realloc(void* ptr, size_t size) {
    void* newptr;
    size_t oldsize = GET_SIZE(HDRP(ptr));

    // If the pointer is NULL, equivalent to mm_malloc
    if (ptr == NULL)
        return mm_malloc(size);

    // If the new size is equal to the old size, return the original pointer
    if (oldsize == size)
        return ptr;

    // If the new size is zero, free the block and return NULL
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    // Allocate a new block with the requested size
    if (!(newptr = mm_malloc(size))) {
        return NULL;
    }

    // Determine the size to copy (minimum of old and new size)
    if (size < oldsize)
        oldsize = size;

    // Copy data from the old block to the new block
    memcpy(newptr, ptr, oldsize);

    // Free the old block
    mm_free(ptr);

    // Return the new pointer
    return newptr;
}

static void* coalesce(void* bp) {
    // Get allocation status of the previous and next blocks
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(FTRP(NEXT_BLKP(bp)));

    // Get the size of the current block
    size_t size = GET_SIZE(FTRP(bp));

    // Case 1: Both previous and next blocks are allocated
    if (prev_alloc && next_alloc) {
        return bp;  // No coalescing needed, return the original block pointer
    }
    // Case 2: Previous block is allocated, but next block is free
    else if (prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));  // Combine with the next block
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    // Case 3: Previous block is free, but next block is allocated
    else if (!prev_alloc && next_alloc) {
        size +=
            GET_SIZE(HDRP(PREV_BLKP(bp)));  // Combine with the previous block
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        bp = PREV_BLKP(
            bp);  // Update block pointer to the start of the combined block
    }
    // Case 4: Both previous and next blocks are free
    else {
        size += (GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp))));
        // Combine with both previous and next blocks
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(
            bp);  // Update block pointer to the start of the combined block
    }

    // Return the updated block pointer after coalescing
    return bp;
}

static void place(void* bp, size_t asize) {
    size_t size = GET_SIZE(HDRP(bp));

    // Check if there is enough space to split the block
    if ((size - asize) >= 2 * DSIZE) {
        // Allocate the requested portion
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));

        // Move the pointer to the next block and set its header and footer
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(size - asize, 0));
        PUT(FTRP(bp), PACK(size - asize, 0));
    } else {
        // Allocate the entire block if splitting is not practical
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
    }
}

static void* first_fit(size_t asize) {
    void* bp = heap_listp;  // Initialize pointer to the start of the heap
    size_t size;

    // Iterate through the blocks in the heap until the end is reached
    while ((size = GET_SIZE(HDRP(bp))) != 0) {
        // Check if the block is unallocated and has sufficient size
        if (size >= asize && !GET_ALLOC(HDRP(bp)))
            return bp;  // Return the pointer to the found block

        // Move to the next block in the heap
        bp = NEXT_BLKP(bp);
    }

    // Return NULL if no suitable block is found
    return NULL;
}

static void* best_fit(size_t asize) {
    void* bp = heap_listp;  // Initialize pointer to the start of the heap
    size_t size;
    void* best = NULL;    // Pointer to the best-fit block
    size_t min_size = 0;  // Minimum size found among eligible blocks

    // Iterate through the blocks in the heap until the end is reached
    while ((size = GET_SIZE(HDRP(bp))) != 0) {
        // Check if the block is unallocated, has sufficient size, and is a
        // better fit
        if (size >= asize && !GET_ALLOC(HDRP(bp)) &&
            (min_size == 0 || min_size > size)) {
            // Update the minimum size and best-fit block pointer
            min_size = size;
            best = bp;
        }

        // Move to the next block in the heap
        bp = NEXT_BLKP(bp);
    }

    // Return the pointer to the best-fit block (NULL if none found)
    return best;
}
