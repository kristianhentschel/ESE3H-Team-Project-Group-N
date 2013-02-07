#ifndef __SYSPARAMDEFNS
#define __SYSPARAMDEFNS

/*
 * Author: Peter Dickman
 * Created: 15-11-95
 * Version 1.01
 */

#define SIMPLIFY_VMPROBLEM

#ifdef SIMPLIFY_VMPROBLEM

/* Can reduce the volume of our datastructures by reducing parameters */

#define MAX_PROCESSES                   6
#define MAX_THREADS                    15
#define PAGE_SIZE                      512
#define BYTES_IN_ADDRESSABLE_LOCATION   4
#define MAX_PAGES                      1024
#define MIN_PHYSICAL_MEMORY             1
#define MAX_PHYSICAL_MEMORY            1024

#else

/* Limit the maximum number of threads in the system to 4096
 * and the maximum number of processes to 512
 */

#define MAX_PROCESSES  512
#define MAX_THREADS   4096

/* Have pages of 1024 longwords i.e. 4192 bytes 
 * Use longword-addressing (i.e. each address indicates a longword)
 *
 * an address space can contain at most 2^20 pages
 * i.e. limit address spaces to 4 gigabyte 
 */

#define PAGE_SIZE                      1024
#define BYTES_IN_ADDRESSABLE_LOCATION     4
#define MAX_PAGES                     (1024*1024)

/* Limited amount of physical memory available, can be adjusted
 * at start-up time within these limits (units are pages)
 */

#define MIN_PHYSICAL_MEMORY    1
#define MAX_PHYSICAL_MEMORY (512)

#endif


/* Tells us how many pages are actually available    */
/* answer must lie within the bounds specified above */

int get_PM_size(void);


#endif




