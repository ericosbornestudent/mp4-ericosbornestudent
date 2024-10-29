/* crowd_support.h 
 * Eric Osborne
 * ejo
 * ECE 2230 Fall 2024 
 * MP2
 *
 * Propose: A template for MP3
 *
 * Assumptions: structure definitions and public functions as defined for
 *              assignmnet.  There are no extensions.
 *
 *              All function prototypes in this file must start with the prefix
 *              crowd_ and are public.  
 *
 * Bugs:
 *
 * You may change this file if needed for your design.
 */

#define MAXLINE 128

/* prototype function definitions */

/* function to compare service records */
int crowd_rank_depts(const service_info_t *rec_a, const service_info_t *rec_b);

/* functions to create and cleanup a list */
linked_list_t *crowd_create_tasklist(void);
void crowd_cleanup(linked_list_t *);

/* Functions to get and print service information */
service_info_t *crowd_build_service_record(int dept_id); /* collect input from user */
void crowd_print_service_info(service_info_t *rec); /* print one record */
void crowd_print_list(linked_list_t *list_ptr, const char *);      /* print list of records */

void crowd_req(linked_list_t *list_ptr, int dept_id);
void crowd_process(linked_list_t *fifo_ptr, linked_list_t *task_ptr, int task_max_size);
void crowd_assign(linked_list_t *task_ptr, int dept_id);
void crowd_find(linked_list_t *fifo_ptr, linked_list_t *task_ptr, int dept_id);
void crowd_stats(linked_list_t *fifo, linked_list_t *task, int task_max_size);

// added for mp3
void crowd_list_sort(linked_list_t *list_ptr, int sort_type, int(*fcomp)(const mydata_t *, const mydata_t *));

/* commands specified to vim. ts: tabstop, sts: soft tabstop sw: shiftwidth */
/* vi:set ts=8 sts=4 sw=4 et: */
