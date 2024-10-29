/* linked_list_lib.c
 * Eric Osborne
 * ejo
 * ECE 2230 Fall 2024
 * MP3
 *
 * Propose: This file contains methods to manipulate 2 way linked lists
 *
 * Assumptions: none
 * 
 * Bugs: none found
 */
#define _GNU_SOURCE             // this is needed for qsort
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include "linked_list_lib.h"    // function prototypes and definitions

#define TRUE  1
#define FALSE 0

// prototypes for private functions used in linked_list.c only
void list_debug_validate(linked_list_t *L);

/* ----- below are the functions  ----- */

/* Allocates a new, empty list 
 *
 * If the comparison function is NULL, then the list is unsorted.
 *
 * Otherwise, the list is initially assumed to be sorted.  Note that if 
 * linked_list_insert is used the list is changed to unsorted.  
 *
 * Use linked_destruct to remove and deallocate all elements on a list 
 * and the header block.
 *
 * (This function is written and no changes needed. It provides an example
 *  of how save the comparison function pointer.  See other examples in this
 *  file.)
 */
linked_list_t * linked_list_construct(int (*compare_function)(const mydata_t *, const mydata_t *))
{
    linked_list_t *L;

    L = (linked_list_t *) malloc(sizeof(linked_list_t));
    L->llhead = NULL;
    L->lltail = NULL;
    L->llcount = 0;
    L->comp_proc = compare_function;
    if (compare_function == NULL)
        L->llsortedstate = FALSE;
    else
        L->llsortedstate = TRUE;

    /* the last line of this function must call validate */
    //list_debug_validate(L);
    return L;
}

/* Deallocates the contents of the specified list, releasing associated memory
 * resources for other purposes.
 *
 * Free all elements in the list and the header block.
 */
void linked_list_destruct(linked_list_t *list_ptr)
{
    // free the individual nodes
    while(list_ptr->llhead != NULL)
    {
        free(list_ptr->llhead->userdata);
        list_ptr->llhead->userdata = NULL;
        list_ptr->lltail = NULL;
        list_ptr->lltail = list_ptr->llhead;
        list_ptr->llhead = list_ptr->llhead->rightlink;
        free(list_ptr->lltail);
        list_ptr->lltail = NULL;
    }
    // free the list
    list_ptr->llhead = NULL;
    list_ptr->lltail = NULL;
    free(list_ptr);
    list_ptr = NULL;
}

/* Obtains a pointer to an element stored in the specified list, at the
 * specified list position
 * 
 * list_ptr: pointer to list-of-interest.  A pointer to an empty list is
 *           obtained from linked_list_construct.
 *
 * pos_index: position of the element to be accessed.  Index starts at 0 at
 *            front of the list, and incremented by one until the back is
 *            reached.  Can also specify LLIST_FRONT and LLIST_BACK
 *
 * return value: pointer to the mydata_t element accessed in the list at the
 * index position.  A value NULL is returned if the pos_index does not 
 * correspond to an element in the list.
 */
mydata_t * linked_list_access(linked_list_t *list_ptr, int pos_index)
{
    assert(list_ptr != NULL);

    /* handle special cases.
     *   1.  The list is empty
     *   2.  Asking for the front 
     *   3.  Asking for the back 
     *   4.  specifying a position that is out of range.  This is not defined
     *       to be an error in this function, but instead it is assumed the 
     *       calling function is looping to find an end of the list 
     */

    // the list is empty
    if (list_ptr->llcount == 0) {
        return NULL;
    }
    else if (pos_index == LLIST_FRONT || pos_index == 0) {
        return list_ptr->llhead->userdata;
    }
    else if (pos_index == LLIST_BACK || pos_index == list_ptr->llcount - 1) {
        return list_ptr->lltail->userdata;
    }
    else if (pos_index < 0 || pos_index >= list_ptr->llcount)
        return NULL;   // does not correspond to position in list

    //new
    llnode_t *crawler = NULL;
    int i=0;
    
    if(pos_index < (list_ptr->llcount)/2)
    {
        crawler = list_ptr->llhead;
        for(i=0; i<pos_index; i++) crawler = crawler->rightlink;
    }else{
        crawler = list_ptr->lltail;
        for(i=0; i<((list_ptr->llcount) - 1-pos_index); i++) crawler = crawler->leftlink;
    }
    //done

    return crawler->userdata;
}

/* Finds an element in a list and returns a pointer to the mydata_t memory
 * block.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * elem_ptr: element against which other elements in the list are compared.
 *           Note: it is required that the comp_proc function pointer is used
 *           to check for a match.  It is found in the linked_list_t header block. 
 *
 * NOTICE: pos_index is returned and is not an input value!
 *
 * The function returns a pointer to the mydata_t memory block for the 
 * llnode_t that contains the first matching element if a match if found.  
 * If a match is not found the return value is NULL.
 *
 * The function also returns the integer position of matching element with the
 *           lowest index.  If a matching element is not found, the position
 *           index that is returned should be -1. 
 *
 * pos_index: used as a return value for the position index of matching element
 */
mydata_t * linked_list_elem_find(linked_list_t *list_ptr, mydata_t *elem_ptr, int *pos_index)
{
    *pos_index = 0;
    llnode_t *crawler = list_ptr->llhead;
    while(crawler != NULL)
    {
        // return the data and change pos_int to where it was found
        *pos_index = *pos_index + 1;
        if(crawler->userdata->department_id == elem_ptr->department_id)
        {
            return crawler->userdata;
        }
        crawler = crawler->rightlink;
    }

    /* fix the return value and the value of pos_index */
    *pos_index = -1;     //  this value is wrong and you must fix it
    return NULL;
}

/* Inserts the data element into the specified list at the specified
 * position.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * elem_ptr: pointer to the memory block to be inserted into list.
 *
 * pos_index: numeric position index of the element to be inserted into the 
 *            list.  Index starts at 0 at front of the list, and incremented by 
 *            one until the back is reached.  The index can also be equal
 *            to LLIST_FRONT or LLIST_BACK (these are special negative 
 *            values use to provide a short cut for adding to the front 
 *            or back of the list).
 *
 * If pos_index is greater than the number of elements currently in the list, 
 * the element is simply appended to the end of the list (no additional elements
 * are inserted).
 *
 * Note that use of this function results in the list to be marked as unsorted,
 * even if the element has been inserted in the correct position.  That is, on
 * completion of this subroutine the list_ptr->llsortedstate must be equal 
 * to FALSE.
 */
void linked_list_insert(linked_list_t *list_ptr, mydata_t *elem_ptr, int pos_index)
{
    assert(list_ptr != NULL);
    assert(pos_index == LLIST_FRONT || pos_index == LLIST_BACK || pos_index >= 0);

    // construct a new node
    llnode_t *new_node = (llnode_t *)malloc(sizeof(llnode_t));
    new_node->userdata = elem_ptr;

    // redefine the pos_index based on our constants
    if(pos_index == 0) pos_index = LLIST_FRONT;
    if(pos_index == linked_list_count(list_ptr)) pos_index = LLIST_BACK;

    // there are 4 distinct cases which we check for

    // if the list is empty
    if(list_ptr->llhead == NULL)
    {
        list_ptr->llhead = new_node;
        list_ptr->lltail = new_node;
        new_node->leftlink = NULL;
    
        new_node->rightlink = NULL;
    //if the node is at the front
    }else if(pos_index == LLIST_FRONT){
        new_node->rightlink = list_ptr->llhead;
        new_node->leftlink = NULL;
        list_ptr->llhead->leftlink = new_node;
        list_ptr->llhead = new_node;
    // if the node is at the back
    }else if(pos_index == LLIST_BACK){
        new_node->leftlink = list_ptr->lltail;
        new_node->rightlink = NULL;
        if(list_ptr->lltail != NULL) list_ptr->lltail->rightlink = new_node;
        list_ptr->lltail = new_node;
    // the node is somewhere between the nodes
    }else{
        llnode_t *crawler = NULL;
        int i=0;
        
        if(pos_index < (list_ptr->llcount)/2)
        {
            crawler = list_ptr->llhead;
            for(i=0; i<pos_index; i++) crawler = crawler->rightlink;
        }else{
            crawler = list_ptr->lltail;
            for(i=0; i<((list_ptr->llcount) - 1-pos_index); i++) crawler = crawler->leftlink;
        }
        new_node->rightlink = crawler;
        new_node->leftlink = crawler->leftlink;
        crawler->leftlink->rightlink = new_node;
        crawler->leftlink = new_node;
    }
    list_ptr->llcount = list_ptr->llcount + 1;

    /* the last two lines of this function must be the following */
    if (list_ptr->llsortedstate == TRUE) 
    list_ptr->llsortedstate = FALSE;
}

/* Inserts the element into the specified sorted list at the proper position,
 * as defined by the comp_proc.  This function is defined in the header:
 *     list_ptr->comp_proc(A, B)
 *
 * The comparison procedure must accept two arguments (A and B) which are both
 * pointers to elements of type mydata_t.  The comparison procedure returns an
 * integer code which indicates the precedence relationship between the two
 * elements.  The integer code takes on the following values:
 *    1: A should be closer to the front of the list than B
 *   -1: B should be closer to the front of the list than A
 *    0: A and B are equal in rank
 *
 * If the element to be inserted is equal in rank to an element already
 * in the list, the newly inserted element will be placed _after all_ the
 * elements of equal rank that are already in the list.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * elem_ptr: pointer to the element to be inserted into list.
 *
 * If you use linked_list_insert_sorted, the list preserves its sorted nature.
 *
 * If you use linked_list_insert, the list will be considered to be unsorted, even
 * if the element has been inserted in the correct position.
 *
 * If the list is not sorted and you call linked_list_insert_sorted, this subroutine
 * must generate a system error and the program should immediately stop.
 *
 */
void linked_list_insert_sorted(linked_list_t *list_ptr, mydata_t *elem_ptr)
{

    // the list is always assumed to be sorted already
    llnode_t *rover = list_ptr->llhead;
    //int comp;
    int index = 0;
    while(rover != NULL)
    {
        // i'm retrofiting this function for just userdata
        //comp = list_ptr->comp_proc(elem_ptr,rover->userdata);
        if(elem_ptr->department_id < rover->userdata->department_id)
        {
            linked_list_insert(list_ptr, elem_ptr, index);
            list_ptr->llsortedstate = TRUE;
            return;
        }
        index++;
        rover = rover->rightlink;
    }

    linked_list_insert(list_ptr,elem_ptr,LLIST_BACK);
    list_ptr->llsortedstate = TRUE;
}

/* Removes the element from the specified list that is found at the 
 * specified list position.  A pointer to the data element is returned.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * pos_index: position of the element to be removed.  Index starts at 0 at
 *            front of the list, and incremented by one until the back is
 *            reached.  Can also specify LLIST_FRONT and LLIST_BACK
 *
 * Attempting to remove an element at a position index that is not contained in
 * the list will result in no element being removed, and a NULL pointer will be
 * returned.
 */
mydata_t * linked_list_remove(linked_list_t *list_ptr, int pos_index)
{
    assert(list_ptr != NULL);
    assert(pos_index == LLIST_FRONT || pos_index == LLIST_BACK || pos_index >= 0);

    llnode_t *rover = NULL;
    int i=0;
    // if(1 < 200) start from begining
    // else if(300 > 200) start form end
    if(pos_index < (list_ptr->llcount)/2)
    {
        rover = list_ptr->llhead;
        for(i=0; i<pos_index; i++) rover = rover->rightlink;
    }else{
        rover = list_ptr->lltail;
        int max = list_ptr->llcount-1-pos_index;
        for(i=0; i<max; i++) rover = rover->leftlink;
    }
    //done

    // disconect the node from the list while crossing neighboring connections
    if(list_ptr->llhead == rover) list_ptr->llhead = rover->rightlink;
    if(list_ptr->lltail == rover) list_ptr->lltail = rover->leftlink;
    if(rover->leftlink != NULL) rover->leftlink->rightlink = rover->rightlink;
    if(rover->rightlink != NULL) rover->rightlink->leftlink = rover->leftlink;

    list_ptr->llcount = list_ptr->llcount - 1;
    

    // the rover must be freed and the data perserved
    mydata_t *return_data = rover->userdata;
    free(rover);
    rover = NULL;
    
    return return_data;
}

/* Obtains the length of the specified list, that is, the number of elements
 * that the list contains.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * Returns an integer equal to the number of elements stored in the list.  An
 * empty list has a size of zero.
 *
 * (This function is already written, so no changes necessary.)
 */
int linked_list_count(linked_list_t *list_ptr)
{
    assert(list_ptr != NULL);
    assert(list_ptr->llcount >= 0);
    return list_ptr->llcount;
}

/* This function verifies that the pointers for the two-way linked list are
 * valid, and that the list size matches the number of items in the list.
 *
 * If the linked list is sorted it also checks that the elements in the list
 * appear in the proper order.
 *
 * The function produces no output if the two-way linked list is correct.  It
 * causes the program to terminate and print a line beginning with "Assertion
 * failed:" if an error is detected.
 *
 * The checks are not exhaustive, so an error may still exist in the
 * list even if these checks pass.
 *
 * YOU MUST NOT CHANGE THIS FUNCTION.  WE USE IT DURING GRADING TO VERIFY THAT
 * YOUR LIST IS CONSISTENT.
 */
void list_debug_validate(linked_list_t *L)
{
    llnode_t *N;
    int count = 0;
    assert(NULL != L); 
    if (NULL == L->lltail) 
	assert(NULL == L->llhead && 0 == L->llcount);
    else
	assert(NULL == L->lltail->rightlink);
    if (L->llhead != NULL) 
	assert(L->llhead->leftlink == NULL);
    else
	assert(L->lltail == NULL && L->llcount == 0);
    if (0 == L->llcount) assert(NULL == L->llhead && NULL == L->lltail);
    if (L->llhead == L->lltail && L->lltail != NULL) assert(L->llcount == 1);
    if (1 == L->llcount) {
        assert(L->llhead == L->lltail && L->lltail != NULL);
        assert(NULL == L->lltail->rightlink && NULL == L->lltail->leftlink);
        assert(NULL != L->lltail->userdata);
    }
    assert(L->llsortedstate == TRUE || L->llsortedstate == FALSE);
    if (1 < L->llcount) {
        assert(L->llhead != L->lltail && NULL != L->llhead && NULL != L->lltail);
        N = L->lltail;
        while (N != NULL) {
            assert(NULL != N->userdata);
            if (NULL != N->leftlink) assert(N->leftlink->rightlink == N);
            else assert(N == L->llhead);
            count++;
            N = N->leftlink;
        }
        assert(count == L->llcount);
    }
    if (L->llsortedstate && NULL != L->llhead) {
        N = L->llhead;
        while (N->rightlink != NULL) {
            assert(L->comp_proc(N->userdata, N->rightlink->userdata) != -1);   // A <= B or A !> B
            N = N->rightlink;
        }
    }
}

/* This is a helper functiton that gets the maximum node in a linked list*/
int linked_list_max(linked_list_t *A, int m, int n)   /* assume m<n */
{
    int i=0;
    llnode_t *rover = A->llhead;
    int max = 2147483647;
    int max_index = 0;

   for(i=0; i<m;i++) rover = rover->rightlink;
   for(i=i; i<=n;i++)
   {
        // The list is supposed to be in ascending order so I flipped the
        if(rover->userdata->department_id < max)
        {
            max = rover->userdata->department_id;
            max_index = i;
        }
        rover = rover->rightlink;
   } 
   return max_index;                      /* return j == position of the largest */
}

/* This is a helper function to swaps two nodes*/
void linked_list_swap(linked_list_t *A, int m, int n)
{    
    // if they are both do nothing to be more efficient
    if(m==n) return;

    // ptr_m and ptr_n crawl to the nodes
    llnode_t *ptr_m = NULL;
    llnode_t *ptr_n = NULL;
    int i=0;

    // basically decide if its better to start from the tail or head
    if(m > (A->llcount)/2)
    {
        ptr_m = A->lltail;
        for(i=0; i<((A->llcount) - 1-m); i++) ptr_m = ptr_m->leftlink;
    }else{
        ptr_m = A->llhead;
        for(i=0; i<m; i++) ptr_m = ptr_m->rightlink;
    }

    if(n > (A->llcount)/2)
    {
        ptr_n = A->lltail;
        for(i=0; i<((A->llcount) - 1-n); i++) ptr_n = ptr_n->leftlink;
    }else{
        ptr_n = A->llhead;
        for(i=0; i<n; i++) ptr_n = ptr_n->rightlink;
    }

    // assign a temp pointer as not to loose the data
    mydata_t *temp = ptr_m->userdata;
    // you can then perform the swap
    ptr_m->userdata = ptr_n->userdata;
    ptr_n->userdata = temp;
    list_debug_validate(A);
}

/* Contiuously swaps the position of two different nodes untill they are the same*/
void recursive_selection_sort(linked_list_t *A, int m, int n)
{
    int MaxPosition;           /* MaxPosition is the index of A's biggest item  */
    if (m < n) {               /* if there is more than one number to sort */
   
        /* Let MaxPosition be the index of the largest number in A[m:n] */
        MaxPosition = linked_list_max(A,m,n);
      
        /* Exchange A[m] <--> A[MaxPosition] */
        linked_list_swap(A,m,MaxPosition);

        /* SelectionSort the subarray A[m+1:n] */
        recursive_selection_sort(A, m+1, n);
   }
}

void iterative_selection_sort(linked_list_t *A, int m, int n)
{
    int MaxPosition;

    while (m < n) {
        MaxPosition = linked_list_max(A,m,n);
        linked_list_swap(A,m,MaxPosition);
        m++;
    }
    
}

/* This is a helper function for merge sort
   It takes two linsts and combines them in ascending order*/
void linked_list_combine(linked_list_t *list_ptr, linked_list_t *L, linked_list_t *R)
{
    // untill both lists are empty
    while(L->llhead != NULL || R->llhead != NULL)
    {
        // if the L list is empty take from the R list
        if(L->llhead == NULL) linked_list_insert(list_ptr,linked_list_remove(R,0),LLIST_BACK);
        // if the R list is empty take from the L lists
        else if(R->llhead == NULL) linked_list_insert(list_ptr,linked_list_remove(L,0),LLIST_BACK);
        // if both lists have one node compare the two and add the one that is greater to the list_ptr
        else if(L->llhead->userdata->department_id < R->llhead->userdata->department_id)
        {
            linked_list_insert(list_ptr,linked_list_remove(L,0),LLIST_BACK);
        }else{
            linked_list_insert(list_ptr,linked_list_remove(R,0),LLIST_BACK);
        }
    }
    // destruct the L and R list but null out their llhead and tails as the nodes
    // are now members of the list_ptr list so dont free these 
    L->llhead = NULL;
    L->lltail = NULL;
    R->llhead = NULL;
    R->lltail = NULL;
    L->llcount = 0;
    R->llcount = 0;
    linked_list_destruct(L);
    linked_list_destruct(R);
}

/* Splits a list into two and calls itself again untill both lists are sorted
   Once both are sorted the lists are combined in ascending order*/
void merge_sort(linked_list_t *list_ptr)
{
    if(list_ptr->llhead->rightlink != NULL)
    {
        linked_list_t *L = linked_list_construct(NULL);
        linked_list_t *R = linked_list_construct(NULL);

        // split the list in two
        int i=0;
        int breakpoint = (list_ptr->llcount)/2;
        int length = list_ptr->llcount;
        for(i=0; i<breakpoint; i++) 
        {
            linked_list_insert(L,linked_list_remove(list_ptr,0),0);
        }
        for(i=breakpoint; i<length; i++) 
        {
            linked_list_insert(R,linked_list_remove(list_ptr,0),0);
        }
        assert(list_ptr->llhead == NULL);
        merge_sort(L);
        merge_sort(R);

        // this will combine the functions in ascending order
        linked_list_combine(list_ptr,L,R);
    }
}
/* Comparison function for qsort */
int qsort_compare(const void *p_a, const void *p_b, void * lptr)
{
    linked_list_t *list_ptr = (linked_list_t *) lptr;
    return list_ptr->comp_proc (*(mydata_t **) p_b, *(mydata_t **) p_a);
}

/* Sort a list by using a modified version of quick sort */
void qsort_linked_list(linked_list_t *list_ptr) 
{
    int i, Asize = linked_list_count(list_ptr);
    assert(list_ptr->comp_proc != NULL);
    mydata_t ** QsortA = (mydata_t **) malloc(Asize*sizeof(mydata_t *));

    // remove all items from list_ptr
    for (i = 0; i < Asize; i++) {
        QsortA[i] = linked_list_remove(list_ptr, LLIST_FRONT);
    }

    // recursive call
    qsort_r(QsortA, Asize, sizeof(mydata_t *), qsort_compare, list_ptr);

    // insert the items back in as they are now sorted
    for (i = 0; i < Asize; i++) {
        linked_list_insert(list_ptr, QsortA[i], LLIST_BACK);
    }
    free(QsortA);
}


// stuff for mp3
void linked_list_sort(linked_list_t *list_ptr, int sort_type, int(*fcomp)(const mydata_t *, const mydata_t *))
{
    if(sort_type == 1)
    {
        // create a new list that is sorted
        linked_list_t *newly_sorted_list = linked_list_construct(NULL);
        int run_for = list_ptr->llcount;

        // basically remove from list and reinsert when sorted
        for(int i=0; i<run_for; i++)
        {
            linked_list_insert_sorted(newly_sorted_list,linked_list_remove(list_ptr,0));
        }

        list_ptr->llhead = newly_sorted_list->llhead;
        list_ptr->lltail = newly_sorted_list->lltail;
        list_ptr->llcount = newly_sorted_list->llcount;
        list_ptr->llsortedstate = newly_sorted_list->llsortedstate;

        newly_sorted_list->llhead = NULL;
        newly_sorted_list->lltail = NULL;
        newly_sorted_list->llcount = 0;

        linked_list_destruct(newly_sorted_list);
        newly_sorted_list = NULL;
    }
    
    if(sort_type == 2) recursive_selection_sort(list_ptr, 0, linked_list_count(list_ptr)-1);
    
    if(sort_type == 3) iterative_selection_sort(list_ptr, 0, linked_list_count(list_ptr)-1);

    if(sort_type == 4) merge_sort(list_ptr);

    if(sort_type == 5) qsort_linked_list(list_ptr);

    list_ptr-> llsortedstate = TRUE;
    // the ONLY list_debug_validate in the program
    list_debug_validate(list_ptr);
}
/* commands for vim. ts: tabstop, sts: softtabstop sw: shiftwidth */
/* vi:set ts=8 sts=4 sw=4 et: */
