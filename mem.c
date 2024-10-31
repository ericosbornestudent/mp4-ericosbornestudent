/* mem.c A template
 * My Name is
 * My User ID is
 * My GitHub ID is
 * Lab4: Dynamic Memory Allocation
 * ECE 2230, Fall 2024
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>

#include "mem.h"

// Global variables required in mem.c only
static int Coalescing = FALSE;      // default FALSE. TRUE if memory returned to free list is coalesced 
static int SearchPolicy = FIRST_FIT;   // default FIRST_FIT.  Can change to BEST_FIT
// NEVER use DummyChunk in your allocation or free functions!!
static mem_chunk_t DummyChunk = {&DummyChunk, 0};
static mem_chunk_t *Rover = &DummyChunk;   // one time initialization

static int NumPages = 0;
static int NumSbrkCalls = 0;

// private function prototypes
void mem_validate(void);
void custom_validate(void);
void Leak_check(void);

/* function to request 1 or more pages from the operating system.
 *
 * new_bytes must be the number of bytes that are being requested from
 *           the OS with the sbrk command.  It must be an integer 
 *           multiple of the PAGESIZE
 *
 * returns a pointer to the new memory location.  If the request for
 * new memory fails this function simply returns NULL, and assumes some
 * calling function will handle the error condition.  Since the error
 * condition is catastrophic, nothing can be done but to terminate 
 * the program.
 */
mem_chunk_t *morecore(int new_bytes) 
{
    char *cp;
    mem_chunk_t *new_p;
    // preconditions that must be true for all designs
    assert(new_bytes % PAGESIZE == 0 && new_bytes > 0);
    assert(PAGESIZE % sizeof(mem_chunk_t) == 0);
    cp = sbrk(new_bytes);
    if (cp == (char *) -1)  /* no space available */
        return NULL;
    new_p = (mem_chunk_t *) cp;
    // Code to count the number of calls to sbrk, and the number of 
    // pages that have been requested
    NumSbrkCalls++; 
    NumPages += new_bytes/PAGESIZE;

    // i did that!!
    new_p->size_units = new_bytes/sizeof(mem_chunk_t);
    return new_p;
}

/* A function to change default operation of dynamic memory manager.
 * This function should be called before the first call to Mem_alloc.
 *
 * search_type: sets Search Policy to BEST_FIT (defults to FIRST_FIT)
 *
 * coalescing_state: sets Coalescing to TRUE (defaults to FALSE)
 */
void Mem_configure(int coalescing_state, int search_type)
{
    assert(coalescing_state == TRUE || coalescing_state == FALSE);
    assert(search_type == FIRST_FIT || search_type == BEST_FIT);
    Coalescing = coalescing_state;
    SearchPolicy = search_type;
}

/* deallocates the space pointed to by return_ptr; it does nothing if
 * return_ptr is NULL.  
 *
 * This function assumes that the rover pointer has already been 
 * initialized and points to some memory block in the free list.
 */
void Mem_free(void *return_ptr)
{
    
    // precondition
    assert(Rover != NULL && Rover->next != NULL);

    // two cases: if coalescing or not
    if (Coalescing == FALSE) {
        /* Background
            To coalesce means to combine blocks that could be physically combined
            Here we dont want to do that so we can just add the block of memory
            dirrectly back into some random point on the free list
        */
       mem_chunk_t *previous_rover_next = Rover->next;
       mem_chunk_t *return_ptr_chunk = (void*)return_ptr - sizeof(mem_chunk_t);
       Rover->next = return_ptr_chunk;
       return_ptr_chunk->next = previous_rover_next;
       custom_validate();
    } else {
        // step 1: loop to find before and after location
        mem_chunk_t *original_rover_position = Rover;
        mem_chunk_t *inserted_block = (void*)return_ptr - sizeof(mem_chunk_t);
        do{
            if(Rover <= inserted_block && (inserted_block < Rover->next || Rover->next->size_units == 0))
            {
                // step 2: insert between before and after and coalesce if possible
                // check if we can combine with the left block
                if(((void*)Rover + (Rover->size_units * sizeof(mem_chunk_t)) == inserted_block))
                {
                    Rover->size_units = Rover->size_units + inserted_block->size_units;
                    // check if we need to combine this new combined block with the left block
                    if(((void*)inserted_block+(inserted_block->size_units*sizeof(mem_chunk_t)) == (void*)Rover->next))
                    {
                        Rover->size_units = Rover->size_units + Rover->next->size_units;
                        Rover->next = Rover->next->next;
                    }
                }else{
                    // check if we can combine our block with the one on the right
                    if(((void*)inserted_block+(inserted_block->size_units*sizeof(mem_chunk_t)) == (void*)Rover->next))
                    {
                        inserted_block->size_units = inserted_block->size_units + Rover->next->size_units;
                        inserted_block->next = Rover->next->next;
                        Rover->next = inserted_block;
                    }else{
                        // this means we couldn't combine any blocks so we just insert it in the lsit
                        inserted_block->next = Rover->next;
                        Rover->next = inserted_block;
                    }
                }
            break;
            }
            Rover = Rover->next;
        }while(Rover != original_rover_position);
    }
    custom_validate();
    
}

/* Context for Splice_block
    This function splices the block if needed and alters the freelist
    It peforms all the necessary asserts and returns the user's writable
    block of memory
*/
void *Splice_block(const int nunits)
{
    custom_validate();
    
    assert(Rover != NULL);
    // requested memory plus one for the unit header
    int requested_units = nunits;

    // Exact match case:
    if(Rover->next->size_units == requested_units)
    {
        custom_validate();
        // This is an exact match which means remove the entire block from the list
        mem_chunk_t *p = Rover->next;

        // remove p from list
        Rover->next = Rover->next->next;

        // NULL next of return as it is good practice
        //p->next = NULL;

        //assert((p->size_units-1)*sizeof(mem_chunk_t) >= nbytes);
        //assert((p->size_units-1)*sizeof(mem_chunk_t) < nbytes + sizeof(mem_chunk_t));
        //assert(p->next == NULL);  // saftey first!
        custom_validate();
        
        return (void*)p+sizeof(mem_chunk_t);
    }

    custom_validate();
    // Splice block case:
    mem_chunk_t *original_block = Rover->next;
    
    // Splice the block creating a new one
    mem_chunk_t *new_block = (void*)original_block + original_block->size_units*sizeof(mem_chunk_t)-requested_units*sizeof(mem_chunk_t);
    original_block->size_units = original_block->size_units - requested_units;
    
    // size of newblock = how many bytes were requested + the header
    new_block->size_units = requested_units;

    mem_chunk_t *p = new_block;
    p->next = NULL;

    //assert((p->size_units-1)*sizeof(mem_chunk_t) >= nbytes);
    //assert((p->size_units-1)*sizeof(mem_chunk_t) < nbytes + sizeof(mem_chunk_t));
    assert(p->next == NULL);  // saftey first!
    custom_validate();
    
    return (void*)p+sizeof(mem_chunk_t);
}

/* returns a pointer to space for an object of size nbytes, or NULL if the
 * request cannot be satisfied.  The memory is uninitialized.
 *
 * This function assumes that there is a Rover pointer that points to
 * some item in the free list.  
 */
void *Mem_alloc(const int nbytes)
{
    
    int nunits = nbytes/sizeof(mem_chunk_t) + 1;
    if(nbytes % sizeof(mem_chunk_t) != 0) nunits++;

    // precondition
    mem_validate();
    assert(nbytes > 0);
    assert(Rover != NULL && Rover->next != NULL);

    if(SearchPolicy == FIRST_FIT)
    {
        /* Background
            For first fit we access and splice the first avaliable chunk of memory
            For best fit we find the block of memory that closest fits our
            For each method you need to splice the block if its too big
        */
        mem_chunk_t *original_rover_position = Rover;
       do{
            if(Rover->next->size_units >= nunits)
            {
                /* Context for Splice_block
                    This function splices the block if needed and alters the freelist
                    It peforms all the necessary asserts and returns the user's writable
                    block of memory
                */
                return Splice_block(nunits);
            }
            Rover = Rover->next;
       }while(Rover != original_rover_position);

       /*
            If it gets here it can be assumed that there is not a suitable block on
            the free list. As such we will request a new block with morecore.
       */
        int newbytes = (((nunits * sizeof(mem_chunk_t)) / PAGESIZE) + ((nunits * sizeof(mem_chunk_t)) % PAGESIZE != 0)) * PAGESIZE; 


        assert(newbytes % PAGESIZE == 0);
        mem_chunk_t *requested_block = morecore(newbytes);
        requested_block->size_units = newbytes/sizeof(mem_chunk_t);
        Mem_free((void*)requested_block+sizeof(mem_chunk_t));
        
        // run the function again now that there is adequate space on the freelist
        return Mem_alloc(nbytes);
    }else{
        /*Background
            This is the best fit case which means we will find a block
            as close to ours as possible even if it means scanning the entire list
        */
       
       mem_chunk_t *original_rover_position = Rover;
       mem_chunk_t *best_fit_block = NULL;
       mem_chunk_t *block_before_best_fit_block = NULL;
       int best_size = INT_MAX;
       do{
            if((Rover->next->size_units >= nunits) && Rover->next->size_units < best_size)
            {
                best_fit_block = Rover->next;
                block_before_best_fit_block = Rover;
                best_size = Rover->next->size_units;
            }
            Rover = Rover->next;

       }while(Rover != original_rover_position);

       if(best_fit_block == NULL)
       {
        // this means that we couldnt find a suitable block so well need to request one
        int newbytes = (
            ((nunits * sizeof(mem_chunk_t)) / PAGESIZE) 
            + ((nunits * sizeof(mem_chunk_t)) % PAGESIZE != 0)) * PAGESIZE; 

        assert(newbytes % PAGESIZE == 0);

        mem_chunk_t *requested_block = morecore(newbytes);
        requested_block->size_units = newbytes/sizeof(mem_chunk_t);
        Mem_free((void*)requested_block+sizeof(mem_chunk_t));
        // run the function again now that there is adequate space on the freelist
        
        return Mem_alloc(nbytes);

       }else{
        // now that we have an ideal block lets splice it
        Rover = block_before_best_fit_block;
        custom_validate();
        return Splice_block(nunits);
        
       }
    }

    // the program should never reach here if it does it has failed
    assert(1 != 1);
    return NULL;
}

/* prints stats about the current free list
 *
 * -- number of items in the linked list including dummy item
 * -- min, max, and average size of each item (in bytes) but not considering
 *    dummy because its size is zero
 * -- total memory in free list (in bytes)
 * -- number of calls to sbrk and number of pages requested
 *
 * A message is printed if all the memory is in the free list
 */
void Mem_stats(void)
{
    int items_in_free_list = 1;   // count how many blocks are in the free list including dummy
    int bytes_in_free_list = 0;
    double average_block_size = 0; // do not include dummy for avg/min/max
    int min_block_size = INT_MAX;
    int max_block_size = 0;

    mem_chunk_t *original_rover_position = Rover;
    
    do{
        items_in_free_list++;
        bytes_in_free_list+=(Rover->size_units * sizeof(mem_chunk_t));
        if(Rover->size_units < min_block_size && Rover->size_units != 0)
        {
            min_block_size = Rover->size_units;
        }
        if(Rover->size_units > max_block_size)
        {
            max_block_size = Rover->size_units;
        }
        Rover = Rover->next;
    }while(Rover != original_rover_position);

    if(items_in_free_list-1 != 0){
    average_block_size = bytes_in_free_list/(items_in_free_list-1);}

    if(min_block_size == INT_MAX) min_block_size = 0;

    printf("  --- Free list stats ---\n");
    printf("\tCount of items : %d\n", items_in_free_list);
    printf("\tMemory in list : %d (bytes)\n", bytes_in_free_list);
    printf("\t           avg : %g (bytes)\n", average_block_size);
    printf("\t           min : %d (bytes)\n", min_block_size);
    printf("\t           max : %d (bytes)\n", max_block_size);
    printf("\tCalls to sbrk  : %d\n", NumSbrkCalls);
    printf("\tNumber of pages: %d\n", NumPages);
    if (bytes_in_free_list == NumPages * PAGESIZE) {
        printf("  all memory is in the heap -- no leaks are possible\n\n\n\n\n\n\n");
    }else{
        printf("LEAKING DATA:\n%d bytes in freelist while %d bytes were allocated\n",bytes_in_free_list,(NumPages*PAGESIZE));
        if(SearchPolicy == FIRST_FIT)
        { 
            printf("While searching with First Fit\n");
        }else{
            printf("While searching with Best Fit\n");
        }
        if(Coalescing)
        {
            printf("While also Coalescing\n");
        }else{
            printf("While NOT Coalescing\n");
        }
    }
    // if list is empty then just one item in the list.  It is the dummy
    // So min is zero by default.  Do not consider dummy in avg/min/max.
    //assert(min_block_size > 0 || items_in_free_list == 1);
}

void Leak_check(void)
{
    int items_in_free_list = 1;   // count how many blocks are in the free list including dummy
    int bytes_in_free_list = 0;
    //double average_block_size = 0; // do not include dummy for avg/min/max
    int min_block_size = INT_MAX;
    int max_block_size = 0;

    mem_chunk_t *original_rover_position = Rover;
    while(Rover->next != original_rover_position)
    {
        items_in_free_list++;
        bytes_in_free_list+=(Rover->next->size_units * sizeof(mem_chunk_t));
        if(Rover->next->size_units < min_block_size && Rover->next->size_units != 0)
        {
            min_block_size = Rover->next->size_units;
        }
        if(Rover->next->size_units > max_block_size)
        {
            max_block_size = Rover->next->size_units;
        }
        Rover = Rover->next;
    }

    //assert(bytes_in_free_list != NumPages * PAGESIZE);
}
/* print table of memory in free list 
 *
 * The print should include the dummy item in the list 
 *
 * A unit is the size of one mem_chunk_t structure
 */
void Mem_print(void)
{
    // note position of Rover is not changed by this function
    assert(Rover != NULL && Rover->next != NULL);
    mem_chunk_t *p = Rover;
    mem_chunk_t *start = p;
    do {
        // example format.  Modify for your design
        printf("p=%p, size=%d (units), end=%p, next=%p %s\n", 
                p, p->size_units, p + p->size_units, p->next, 
                p->size_units!=0?"":"<-- dummy");
        p = p->next;
    } while (p != start);
}

void custom_validate(void)
{
    mem_chunk_t *original_rover_position = Rover;
    Rover = Rover->next;
    while(Rover != original_rover_position)
    {
        // check that each block has initiated values
        if(Rover->size_units == -420) break;

        // verify that the list is in proper order
        if(Coalescing == TRUE)
        {
            
        }

        Rover = Rover->next;
    }
}

/* This is an experimental function to attempt to validate the free
 * list when coalescing is used.  It is not clear that these tests
 * will be appropriate for all designs.  If your design utilizes a different
 * approach, that is fine.  You do not need to use this function and you
 * are not required to write your own validate function.
 */
void mem_validate(void)
{
    /*
    // note position of Rover is not changed by this function
    assert(Rover != NULL && Rover->next != NULL);
    assert(Rover->size_units >= 0);
    int wrapped = FALSE;
    int found_dummy = FALSE;
    mem_chunk_t *p, *largest, *smallest;

    p = Rover;
    do {
        if (p->size_units == 0) {
            assert(found_dummy == FALSE);
            found_dummy = TRUE;
        } else {
            assert(p->size_units > 0);
        }
        p = p->next;
    } while (p != Rover);
    assert(found_dummy == TRUE);

    if (Coalescing) {
        do {
            if (p >= p->next) {
                // this is not good unless at the one wrap around
                if (wrapped == TRUE) {
                    printf("validate: List is out of order, already found wrap\n");
                    printf("first largest %p, smallest %p\n", largest, smallest);
                    printf("second largest %p, smallest %p\n", p, p->next);
                    assert(wrapped == FALSE);   // stop and use gdb
                } else {
                    wrapped = TRUE;
                    largest = p;
                    smallest = p->next;
                }
            } else {
                assert(p + p->size_units < p->next);
            }
            p = p->next;
        } while (p != Rover);
        assert(wrapped == TRUE);
    }*/
}
/* vi:set ts=8 sts=4 sw=4 et: */

