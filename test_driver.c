#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include "linked_list_lib.h"
#include "mem.h"

// structure for equilibrium driver parameters 
typedef struct {
    int SearchPolicy;
    int Coalescing;
    int Seed;
    int Verbose;
    int EquilibriumTest;
    int WarmUp;
    int Trials;
    int AvgNumInts;
    int RangeInts;
    int SysMalloc;
    int UnitDriver;
} driver_params;

// prototypes for functions in this file only 
void getCommandLine(int argc, char **argv, driver_params *dp);
void equilibriumDriver(driver_params *ep);

int main(int argc, char **argv)
{
    driver_params dprms;
    getCommandLine(argc, argv, &dprms);

    // The major choices: search policy and coalescing option 
    if (dprms.SearchPolicy == BEST_FIT) printf("Best-fit search policy");
    else if (dprms.SearchPolicy == FIRST_FIT) printf("First-fit search policy");
    else {
        fprintf(stderr, "Error with undefined search policy\n");
        exit(1);
    }
    if (dprms.Coalescing == TRUE) printf(" using coalescing\n");
    else if (dprms.Coalescing == FALSE) printf(" without coalescing\n");
    else {
        fprintf(stderr, "Error specify coalescing policy\n");
        exit(1);
    }
    Mem_configure(dprms.Coalescing, dprms.SearchPolicy);

    if (dprms.UnitDriver == 0)
    {
        // unit driver 0: basic test with one allocation and free
        printf("\n----- Begin unit driver 0 -----\n");
        char *string;
        const char msg[] = "hello world 15c";
        int len = strlen(msg);
        // add one for null character at end of string 
        string = (char *) Mem_alloc((len+1) * sizeof(char));
        strcpy(string, msg);
        printf("string length=%d\n\"%s\"\n", len, string);
        printf("\nFree list after first allocation\n");
        Mem_stats();
        Mem_print();
        Mem_free(string);
        printf("\nFree list after first free\n");
        printf("unit driver 0 has returned all memory to free list\n");
        Mem_stats();
        Mem_print();
        string = NULL;
        printf("\n----- End unit driver 0 -----\n");
    }
    else if (dprms.UnitDriver == 1)
    {
        printf("\n----- Begin unit driver 1 -----\n");
        /* You MUST create at least one new unit driver.

         Here is an example of a unit test driver.

         This is a specific example of the general statement made in the
         MP4.pdf file, in the section "Testing and performance evaluation".
         This test makes three allocations from the free list with the goal
         of making the third allocation the correct size so that the free
         list is left empty. 
         */

        int unit_size = sizeof(mem_chunk_t);
        int units_in_first_page = PAGESIZE/unit_size;  
        assert((units_in_first_page) * unit_size == PAGESIZE);
        printf("There are %d units in first page, and one unit is %d bytes\n", 
                units_in_first_page, unit_size); 

        int *p1, *p2, *p3, *p4;
        int num_bytes_1, num_bytes_2, num_bytes_3;
        int num_bytes_4;

        // allocate 1st pointer to 1/8 of a page
        num_bytes_1 = ((units_in_first_page)/8 - 1)*unit_size;
        p1 = (int *) Mem_alloc(num_bytes_1);
        printf("first: %d bytes (%d units) at p=%p \n", 
                num_bytes_1, num_bytes_1/unit_size, p1);
        Mem_print();

        // allocate for 2nd pointer to 1/2 of a page
        num_bytes_2 = ((units_in_first_page)/2 - 1)*unit_size;
        p2 = (int *) Mem_alloc(num_bytes_2);
        printf("second: %d bytes (%d units) at p=%p \n", 
                num_bytes_2, num_bytes_2/unit_size, p2);
        Mem_print();

        // allocate remaining memory in free list
        num_bytes_3 = units_in_first_page - num_bytes_1/unit_size 
            - num_bytes_2/unit_size - 3;
        num_bytes_3 *= unit_size;
        p3 = (int *) Mem_alloc(num_bytes_3);
        printf("third: %d bytes (%d units) at p=%p \n", 
                num_bytes_3, num_bytes_3/unit_size, p3);
        Mem_print();
        printf("\n\tunit driver 1: above Mem_print should show an empty free list\n");

        // allocate for 4th pointer to 1/4 a page when free list is empty
        num_bytes_4 = (units_in_first_page/4 - 1)*unit_size;
        p4 = (int *) Mem_alloc(num_bytes_4);
        printf("fourth: %d bytes (%d units) at p=%p \n", 
                num_bytes_4, num_bytes_4/unit_size, p4);
        Mem_print();

        // next put the memory back into the free list:

        printf("\nfirst free of 1/8 a page p=%p \n", p1);
        Mem_free(p1);
        Mem_print();

        printf("second free of 3/8 a page p=%p \n", p3);
        Mem_free(p3);
        Mem_print();

        printf("third free of 1/2 a page p=%p \n", p2);
        Mem_free(p2);
        Mem_print();
        printf("fourth free of 1/4 a page p=%p\n", p4);
        Mem_free(p4);
        printf("unit driver 1 has returned all memory to free list.  Veryify no leak\n");
        Mem_print();
        Mem_stats();
        printf("\n----- End unit test driver 1 -----\n");
    }
    else if (dprms.UnitDriver == 2)
    {
        printf("\n----- Begin unit driver 2 -----\n");
        /* This driver repeats unit driver 1, and then repeats the
         * allocation requests.  Allows for testing of exact fit case.
         *
         * Also tests for best fit, but best fit depends on order blocks
         * are found in the free list.
         */

        int unit_size = sizeof(mem_chunk_t);
        int units_in_first_page = PAGESIZE/unit_size;  
        assert((units_in_first_page) * unit_size == PAGESIZE);

        int *p1, *p2, *p3, *p4;
        int num_bytes_1, num_bytes_2, num_bytes_3;
        int num_bytes_4;

        // allocate 1st pointer to 1/8 of a page
        num_bytes_1 = ((units_in_first_page)/8 - 1)*unit_size;
        p1 = (int *) Mem_alloc(num_bytes_1);

        // allocate for 2nd pointer to 1/2 of a page
        num_bytes_2 = ((units_in_first_page)/2 - 1)*unit_size;
        p2 = (int *) Mem_alloc(num_bytes_2);

        // allocate remaining memory in free list
        num_bytes_3 = units_in_first_page - num_bytes_1/unit_size
            - num_bytes_2/unit_size - 3;
        num_bytes_3 *= unit_size;
        p3 = (int *) Mem_alloc(num_bytes_3);

        // allocate for 4th pointer to 1/4 a page when free list is empty
        num_bytes_4 = (units_in_first_page/4 - 1)*unit_size;
        p4 = (int *) Mem_alloc(num_bytes_4);

        // next put the memory back into the free list:

        Mem_free(p1);
        Mem_free(p3);
        Mem_free(p2);
        Mem_free(p4);
        printf("unit driver 2 has returned all memory to free list.  Should be similar to unit 1\n");
        Mem_print();
        Mem_stats();
        printf("\nRepeat the 4 allocation requests.  With best fit should find exact match for all.\n");
        p1 = (int *) Mem_alloc(num_bytes_1);
        p4 = (int *) Mem_alloc(num_bytes_4);
        p3 = (int *) Mem_alloc(num_bytes_3);
        p2 = (int *) Mem_alloc(num_bytes_2);
        printf("free list after repeating the four mallocs.  No carving if best fit\n");
        Mem_print();
        Mem_stats();
        Mem_free(p2);
        Mem_free(p3);
        Mem_free(p4);
        Mem_free(p1);
        printf("\nunit driver 2 has returned all memory to free list\n");
        Mem_print();
        Mem_stats();

        printf("\nRepeat the 4 allocation requests but with slightly smaller sizes.\n");
        printf("With best fit should leave blocks of sizes 1, 2, 3, and 4.\n");
        p1 = (int *) Mem_alloc(num_bytes_1 - unit_size);
        p2 = (int *) Mem_alloc(num_bytes_2 - 2 * unit_size);
        p3 = (int *) Mem_alloc(num_bytes_3 - 3 * unit_size);
        p4 = (int *) Mem_alloc(num_bytes_4 - 4 * unit_size);
        printf("free list after repeating the four mallocs\n");
        Mem_print();
        Mem_stats();
        Mem_free(p1);
        Mem_free(p4);
        Mem_free(p3);
        Mem_free(p2);
        printf("\nunit driver 2 has returned all memory to free list. Verify no leaks\n");
        Mem_print();
        Mem_stats();

        printf("\n----- End unit test driver 2 -----\n");
    }
    else if (dprms.UnitDriver == 3)
    {
        printf("Begining custom test:\n");
        // string testing
        int len = 100;
        char *string = (char *) Mem_alloc((len+1) * sizeof(char));
        string = "hello world";
        Mem_free(string);
        string = NULL;
    }


    // performance test
    if (dprms.EquilibriumTest)
        equilibriumDriver(&dprms);

    exit(0);
}


/* read in command line arguments.  
 *
 * -u 0  is for the unit drivers, starting with driver 0
 *
 * Other command line arguments are for the equilibrium driver parameters.
 */
void getCommandLine(int argc, char **argv, driver_params *dp)
{
    /* The geopt function creates three global variables:
     *    optopt--if an unknown option character is found
     *    optind--index of next element in argv 
     *    optarg--argument for option that requires argument 
     *
     * The third argument to getopt() specifies the possible argument flags
     *   If an argument flag is followed by a colon, then an argument is 
     *   expected. E.g., "x:y" means -x must have an argument but not -y
     */
    int c;
    int index;


    dp->SearchPolicy = FIRST_FIT;
    dp->Coalescing = FALSE;
    dp->Seed = 10282024;
    dp->Verbose = FALSE;
    dp->EquilibriumTest = FALSE;
    dp->WarmUp = 1000;
    dp->Trials = 100000;
    dp->AvgNumInts = 128;
    dp->RangeInts = 127;
    dp->SysMalloc = FALSE;
    dp->UnitDriver = -1;

    while ((c = getopt(argc, argv, "w:t:s:a:r:f:u:cdve")) != -1) {
        switch(c) {
            case 'u': dp->UnitDriver = atoi(optarg);   break;
            case 'w': dp->WarmUp = atoi(optarg);       break;
            case 't': dp->Trials = atoi(optarg);       break;
            case 's': dp->Seed = atoi(optarg);         break;
            case 'a': dp->AvgNumInts = atoi(optarg);   break;
            case 'r': dp->RangeInts = atoi(optarg);    break;
            case 'd': dp->SysMalloc = TRUE;            break;
            case 'v': dp->Verbose = TRUE;              break;
            case 'e': dp->EquilibriumTest = TRUE;      break;
            case 'c': dp->Coalescing = TRUE;           break;
            case 'f':
                  if (strcmp(optarg, "best") == 0)
                      dp->SearchPolicy = BEST_FIT;
                  else if (strcmp(optarg, "first") == 0)
                      dp->SearchPolicy = FIRST_FIT;
                  else {
                      fprintf(stderr, "invalid search policy: %s\n", optarg);
                      exit(1);
                  }
                  break;
            case '?':
                  if (isprint(optopt))
                      fprintf(stderr, "Unknown option %c.\n", optopt);
                  else
                      fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
            default:
                  printf("Lab4 command line options\n");
                  printf("General options ---------\n");
                  printf("  -v        turn on verbose prints (default off)\n");
                  printf("  -s 54321  seed for random number generator\n");
                  printf("  -c        turn on coalescing (default off)\n");
                  printf("  -f best|first\n");
                  printf("            search policy to find memory block (first by default)\n");
                  printf("  -u 0      run unit test driver\n");
                  printf("  -e        run equilibrium test driver\n");
                  printf("\nOptions for equilibrium test driver ---------\n");
                  printf("  -w 1000   number of warmup allocations\n");
                  printf("  -t 100000 number of trials in equilibrium\n");
                  printf("  -a 128    average size of interger array\n");
                  printf("  -r 127    range for average size of array\n");
                  printf("  -d        use system malloc/free instead of MP4 versions\n");
                  exit(1);
        }
    }
    for (index = optind; index < argc; index++)
        printf("Non-option argument %s\n", argv[index]);
}

/* vi:set ts=8 sts=4 sw=4 et: */
