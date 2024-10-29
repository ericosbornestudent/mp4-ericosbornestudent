/* datatypes.h 
 * Eric Osborne
 * ejo
 * ECE 2230 Fall 2024 
 * MP3
 *
 * Propose: The data type that is stored in the list ADT is defined here.  We
 * define a single mapping that allows the list ADT to be defined in terms of a
 * generic mydata_t.
 *
 * Assumptions: 
 *
 * mydata_t: The type of data that we want to store in the list
 *
 * Bugs:
 */

#define USIZE 8

typedef struct service_info_tag {
    int department_id;	// Department ID
    char username[USIZE]; // username
    int ip_address;	// IP address for department
    int location_code;	// department building location code 
    int authenticated;	// true or false 
    int password;	// 0 if password known.  1 if password needs to be reset 
    int security_key   ;// A single letter indicated type of key 
                        //   Convert letter to integer with a=1, b=2, etc. 
    int time_received;	// time in seconds that information last updated
    int order_received; // set by get_service_info function. Do not change
    int ticket_number;  // set by strike_assign function. Do not change
} service_info_t;

/* the list ADT works on packet data of this type */
typedef service_info_t mydata_t;

/* commands for vim. ts: tabstop, sts: soft tabstop sw: shiftwidth */
/* vi:set ts=8 sts=4 sw=4 et: */
