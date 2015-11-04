/*Hao Yang andrew:haoyang
 * mm.c
 *For my malloc i used an segregated fit to seperate all the sizes into
 *different size classes, each size class would then correspond to
 * the index in a arraylist, since we couldnt use structs to make
 *linked lists or anything. The index would be the block pointer for
 *the currentblock.The block pointer would then allow you to see the
 * previous and next blocks by adding DSIZE. I also made my lists first fit
 * so instead of searching through everything it stops at the first
 * block big enough
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/*
Basic Constants and Macros
*/
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)

// Max values of two vars
#define MAX(x,y) ((x)> (y)? (x) : (y))

//Or the size and the header into a word
#define PACK(size,alloc) ((size)|(alloc))

//gets and puts the works at address
#define GET(p)  (*(unsigned int *)(p))
#define PUT(p,val) (*(unsigned int *)(p) = (val))

//gets the size starting from addr p
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

//gets add or head and footer from the basepointer
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

//nexts and prev blocks from the base pointer
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* If you want debugging output, use the following macro.  When you hand
 * in, remove the #define DEBUG line. */
#define DEBUG
#ifdef DEBUG
# define dbg_printf(...) printf(__VA_ARGS__)
#else
# define dbg_printf(...)
#endif


/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT-1)) & ~0x7)



#define next_block(bp)  (*(void **)(bp))
#define prev_block(bp)  (*(void **)(bp + DSIZE))

/* $end mallocmacros */

/* Global variables */
static char *heap_listp = 0;  /* Pointer to first block */
static char *free_list[13];   /* Pointer to different segmentations*/
static char *storage[3]; // pointers to prev, bp and next blocks
                        //this is helpful because we know where the bp is
                        //in between functions



static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void removeB();
void mm_checkheap(int lineno);
static int returnClassSize(size_t blksize);


/*
 * Return whether the pointer is in the heap.
 * May be useful for debugging.
 */
static int in_heap(const void *p) {
    return p <= mem_heap_hi() && p >= mem_heap_lo();
}

/*
 * Return whether the pointer is aligned.
 * May be useful for debugging.
 */
static int aligned(const void *p) {
    return (size_t)ALIGN(p) == (size_t)p;
}

//instantiates the arraylist and storage for pointers
static int instantiateFree(){
  //error checking
  if(free_list == NULL){
    return -1;
  }
  else{
   memset(free_list,0,sizeof(free_list));
    }

  if(storage == NULL){
    return -1;
  }

  else{
    memset(storage,0,sizeof(storage));
  }

  return 0;
}

/*
 * mm_init - Initialize the memory manager
 */
int mm_init(void)
{
  /* Create the initial empty heap */
  if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1) {
    return -1;
  }
  PUT(heap_listp, 0);                          /* Alignment padding */
  PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); /* Prologue header */
  PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
  PUT(heap_listp + (3*WSIZE), PACK(0, 1));     /* Epilogue header */
  heap_listp += (2*WSIZE);


  instantiateFree();

  /* Extend the empty heap with a free block of CHUNKSIZE bytes */
  if (extend_heap(CHUNKSIZE/WSIZE) == NULL) {
    return -1;
  }

  return 0;
}
/*
 * mm_malloc - Allocate a block with at least size bytes of payload
 */
void *mm_malloc(size_t size)
{
  size_t asize;
  size_t extendsize; // how much to extend by
  char *bp;

  if (size == 0 || size <= 0) {
    return NULL;
  }


  if (size <= DSIZE) {
    asize = 3*DSIZE;
  }
  else {
    asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
  }


  if ((bp = find_fit(asize)) != NULL) {
    place(bp, asize);
    return bp;
  }


  extendsize = MAX(asize,CHUNKSIZE);
  if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
    return NULL;
  place(bp, asize);

  return bp;
}

/*
 * free
 */
void free (void *ptr) {
    if(!ptr) return;

    size_t sze = GET_SIZE(HDRP(ptr));

    if(sze <= 0){
      return;
    }

    if(!in_heap(ptr)){// check in heap
      return;
    }
    if(!aligned(ptr)){//all dis error checking
      return;
    }

    if(heap_listp == 0 || heap_listp == NULL){
      //NULL really shouldnt happen, but just in case
      mm_init();
    }

    PUT(HDRP(ptr),PACK(sze,0));
    PUT(FTRP(ptr),PACK(sze,0));
    coalesce(ptr);


}

//delete the blocks depends on how the prev and next blocks and
//arranged
static void addDelete(int caseNumber, size_t sze, void *bpp,void* np){
  int number = GET_SIZE(HDRP(bpp));
  int classNumber = returnClassSize(number);
  if(sze == 0){
    return;
  }
   if(!in_heap(bpp)){// check in heap
      return;
    }
    if(!aligned(bpp)){//all dis error checking
      return;
    }

  switch(caseNumber){
  case 2:
    storage[1] = np;
    removeB();

  case 1:
  if (free_list[classNumber] == NULL){

      next_block(bpp) = NULL;
      prev_block(bpp)= NULL;
    free_list[classNumber] = bpp;
  }

 else {
    prev_block(bpp) = NULL;
    next_block(bpp) = free_list[classNumber];
    prev_block(free_list[classNumber]) = bpp;
    free_list[classNumber] = bpp;
  }
  break;

  case 4:

  case 3:

    if (free_list[classNumber] == NULL){
    next_block(bpp) = NULL;
    prev_block(bpp) = NULL;
    free_list[classNumber] = bpp;
  }

 else {
   prev_block(bpp) = NULL;
    next_block(bpp) = free_list[classNumber];
    prev_block(free_list[classNumber]) = bpp;
    free_list[classNumber] = bpp;
  }
     break;

  default:
    //should never happen;
    break; }
}

//returns the case number depending on arrangement of blocks
static int checkCases(void* bp){
  char *p = prev_block(bp);
  char *n = next_block(bp);
   if(!in_heap(bp)){// check in heap
      return -1;
    }
    if(!aligned(bp)){//all dis error checking
      return -1;
    }

  //both free
  if(p == NULL && n == NULL){
    return 0; //  perfect
  }

  else if(p == NULL &&  n!= NULL){
    return 1;
  }

  else if(n == NULL && p != NULL){
    return  2;
  }

  else{
  return -1;
  }
}


//coaleses free bloks
static void *coalesce(void *bp)
{
  size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));


  if (prev_alloc && next_alloc) {  /* Case 1 */
    if(prev_alloc > 0){//makes sure the size is positive
      if(next_alloc>0){
    addDelete(1,size,bp,0);
    return bp;}}
  }

  else if (prev_alloc && !next_alloc) {      /* Case 2 */
    if(prev_alloc > 0){
    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    void* next = NEXT_BLKP(bp);
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size,0));
    addDelete(2,size,bp,next);
    }
  }

  else if (!prev_alloc && next_alloc) {      /* Case 3 */
    if(next_alloc >0){
    bp = PREV_BLKP(bp);
    size += GET_SIZE(HDRP(bp));
    storage[1] = bp;
    removeB();

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    addDelete(3,size,bp,0);
    }
  }

  else {
    /* Case 4 */
    void* prev = PREV_BLKP(bp);
    void* next = NEXT_BLKP(bp);
    storage[1] = next;
    removeB();
    storage[1] = prev;
    removeB();


    size += (GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp))));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
    bp = PREV_BLKP(bp);
    addDelete(4,size,bp,0);

  }
  return bp;
}


/*
 * realloc - you may want to look at mm-naive.c
 */
void *realloc(void *oldptr, size_t size) {
    size_t oldsize;
  void *newptr;

  /* If size == 0 then this is just free, and we return NULL. */
  if(size == 0) {
    free(oldptr);
    return 0;
  }

  /* If oldptr is NULL, then this is just malloc. */
  if(oldptr == NULL) {
    return malloc(size);
  }

  newptr = malloc(size);

  /* If realloc() fails the original block is left untouched  */
  if(!newptr) {
    return 0;
  }

  /* Copy the old data. */
  oldsize = GET_SIZE(HDRP(oldptr));
  if(size < oldsize) oldsize = size;
  memcpy(newptr, oldptr, oldsize);

  /* Free the old block. */
  free(oldptr);

  return newptr;
}

static void place(void *bp, size_t asize)
{

size_t csize = GET_SIZE(HDRP(bp));

 if(bp == NULL){
   return;
 }

 if(!aligned(bp)){
   return;
 }
   if (!GET_ALLOC(bp)){
         storage[1] = bp;
         removeB();
         PUT(HDRP(bp), PACK(asize, 1));
         PUT(FTRP(bp), PACK(asize, 1));
       }

  if ((csize - asize) > 2*DSIZE) {
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));
    bp = NEXT_BLKP(bp);
    storage[1] = bp;
    PUT(HDRP(bp), PACK(csize-asize, 0));
    PUT(FTRP(bp), PACK(csize-asize, 0));
    addDelete(1,asize,bp,0);


  }
  else {

    PUT(HDRP(bp), PACK(csize, 1));
    PUT(FTRP(bp), PACK(csize, 1));
    removeB();
  }


}


/*
Extend heap function;
*/
static void *extend_heap(size_t words){
  char *bp;
  size_t size;

  size = (words %2)?(words + 1)*WSIZE: words * WSIZE;
  if((long)(bp = mem_sbrk(size)) == -1)
    return NULL;

  PUT(HDRP(bp), PACK(size,0));
  PUT(FTRP(bp), PACK(size,0));
  PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1));

  if(bp == NULL){
    return (void*)-1;// error checking
  }
  return coalesce(bp);
}


/*
 * calloc - you may want to look at mm-naive.c
 * This function is not tested by mdriver, but it is
 * needed to run the traces.
 */
void *calloc (size_t nmemb, size_t size) {
  size_t bytes = nmemb * size;
  void *newptr;
  if(size <= 0){//error checking
    return (void*)-1;
  }
  if(nmemb <= 0){
    return (void*)-1;
  }
  newptr = malloc(bytes);
  memset(newptr, 0, bytes);// set as 0

  return newptr;
}



static void *find_fit(size_t asize){
  void *bp = NULL;
   void *bpp = NULL;

  for (int i= 0; i < 13; i++){
      bp = free_list[i];

      //in case the fit is too small, this errors checks
      //and skips it
      if(bp == NULL || GET_SIZE(HDRP(bp)) == 0){
        continue;
      }

      //first fit, it fits we're done, dont care about subsequent fits
      if ((asize <= GET_SIZE(HDRP(bp))) &&
          ((GET_SIZE(HDRP(bp)) - asize) <  (size_t)1<<31)) {
        if(aligned(bp)){//error checking
        bpp = bp;
        storage[1] = bp;
        break;}
      }}

  return bpp;


}




//removes the blocks depending on each case
static void removeB(){
  void* bp = storage[1];
  int index = returnClassSize(GET_SIZE(HDRP(storage[1])));
  storage[0] = prev_block(bp);
  storage[2] = next_block(bp);
  if(!aligned(bp)){
    return;
  }
  if(bp == NULL){
    return;
  }
  if(checkCases(bp) == 0){
    if(GET_SIZE(HDRP(bp)) <= 0){ //error checks
      return;
    }
    if(!in_heap(bp)){
      return;
    }

    free_list[index] = NULL;
    prev_block((size_t)bp) = NULL;
    next_block((size_t)bp+1) = NULL;
    storage[0] = NULL;
    storage[2] = NULL;
  }

  else if(checkCases(bp) == 1){

       free_list[index] = storage[2];
       prev_block(storage[2]) = NULL;
       storage[2] = NULL;
       prev_block(bp) = NULL;

  }

  else if(checkCases(bp) == 2){

     next_block(storage[0]) = NULL;
     next_block((size_t)bp+1) = NULL;
  }

  else if(checkCases(bp) == -1){
    if(GET_SIZE(HDRP(bp)) <= 0){ //error checks
      return;
    }
    if(!in_heap(bp)){
      return;
    }
    prev_block(storage[2]) = storage[0];
    next_block(storage[0]) = storage[2];
    next_block((size_t)bp+1) = NULL;
    prev_block(bp) = NULL;
    storage[1] = NULL;
  }
  else{
    //nothing else
  }


}

/**
segregated free list algo- powers of 2 on "bigger" numbers
everything else is seperated by groups of 64
*/
static int returnClassSize(size_t size){
  if(size <= 64){
    return 0;}
  if(size <= 128){
    return 1;}
  if(size <= (128+64)){
    return 2;}
  if(size <= 256){
    return 3;}
  if(size <= 256+64){
    return 4;}
  if(size <= 256+128){
    return 5;}
  if(size <= 256+128+64){
    return 6;}
  if(size <= 512){
    return 7;}
  //now we sort by powers of 2
  if(size <= 1024){
    return 8;
  }
  if(size <= 2048){
    return 9;}
  else if(size <= 4096){
    return 10;}
  else if(size <= 8192){
    return 11;}
  else//(size > 8192)
    {
    return 12;}
}



/*
 * mm_checkheap
 */
void mm_checkheap(int lineno) {

  char* bp = heap_listp;

  if(!GET_ALLOC(HDRP(heap_listp))){
    printf("Bad Prolougue Header");
  }

  if(GET_SIZE(HDRP(heap_listp)) != 8){
    //prolougue is an 8 byte allocated block
    printf("%d: Prolouge not 8 bytes",lineno);

  }

  for(bp = heap_listp; GET_SIZE(HDRP(bp)) > 0;){
    bp = NEXT_BLKP(bp);

    if(!aligned(bp)){
      printf("Not correct byte size");
    }

    if(!in_heap(bp)){
      printf("Not in heap");
    }


  if(GET_SIZE(HDRP(bp)) != 0){
    //epilogue block is 0 bytes
    printf("Epilogue not 0 bytes");
  }

  if(!GET_ALLOC(HDRP(bp))){
    printf("Bad Epilogue Header");
  }

  return;
  }}
