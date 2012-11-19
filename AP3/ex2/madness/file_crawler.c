#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "ts_fifo.h"
#include "ts_priority.h"
#include "re.h"

#define CRAWLER_THREADS 1

/* recursive method that adds directories to the work queue. */
static int processDirectory(char *dirname, ts_fifo *work_queue, int verbose);

/* convert given bash pattern to regular expression */
static void cvtPattern(char pattern[], const char *bashpat); 

/* the worker thread's main method */
struct worker_args{
	ts_fifo work_queue;
	ts_priority results; 
	char *pattern;
}
static void worker(ts_fifo work_queue, ts_priority results, const char pattern[]);

int main(int argc, char *argc[]) {
	//initialise variables
	int i;
	pthread_t threads[CRAWLER_THREADS];
	struct worker_args *args;

	//parse arguments (taken from Joe's starting code)


	//setup shared data structures
	ts_fifo work_queue = ts_fifo_create();
	ts_fifo results = ts_fifo_create();

	//create and launch worker threads
	if( (args = malloc(sizeof(struct worker_args))) == NULL)
		return 1;
	
	args->work_queue = work_queue;
	args->results = results;
	args->pattern = pattern;
	
	for (i = 0; i < CRAWLER_THREADS; i++) {
		threads[i], NULL, NULL
	}

	//fill work_queue with actual data
	
	//add a suicide command for each thread so they will die when no more work is left.

	//wait for all threads to die
	
	pthread_join();

	//harvest and print results
	while( (char *dir = (char *) ts_priority_dequeue(results)) != NULL ) {
		printf("%s\n", dir);
		free(dir); //TODO
	}


	//destroy shared data structures
	ts_fifo_destroy(work_queue);
	ts_priority_destroy(result);

	free args;
}



static void worker(ts_fifo work_queue, ts_priority results, const char pattern[]) {
	while( *(char *dir = (char *) ts_fifo_dequeue(work_queue)) != '\0' ){
		//do something with this dir - get all children and match them to the regular expression
	}

	fprintf(stderr, "Worker: My work here is done.\n");

	return;
}




/*
 * recursively opens directory files
 *
 * if the directory is successfully opened, it is added to the linked list
 *
 * for each entry in the directory, if it is itself a directory,
 * processDirectory is recursively invoked on the fully qualified name
 *
 * if there is an error opening the directory, an error message is
 * printed on stderr, and 1 is returned, as this is most likely indicative
 * of a protection violation
 *
 * if there is an error duplicating the directory name, or adding it to
 * the linked list, an error message is printed on stderr and 0 is returned
 *
 * if no problems, returns 1
 * 
 * By Joe Sventek, part of the starting files for AP3 Ex 2. Modified to add
 * to a ts_fifo queue instead of a linked list.
 */

static int processDirectory(char *dirname, ts_fifo *work_queue, int verbose) {
   DIR *dd;
   struct dirent *dent;
   char *sp;
   int len, status = 1;
   char d[4096];

   /*
    * eliminate trailing slash, if there
    */
   strcpy(d, dirname);
   len = strlen(d);
   if (len > 1 && d[len-1] == '/')
      d[len-1] = '\0';
   /*
    * open the directory
    */
   if ((dd = opendir(d)) == NULL) {
      if (verbose)
         fprintf(stderr, "Error opening directory `%s'\n", d);
      return 1;
   }
   /*
    * duplicate directory name to insert into linked list
    */
   sp = strdup(d);
   if (sp == NULL) {
      fprintf(stderr, "Error adding `%s' to linked list\n", d);
      status = 0;
      goto cleanup;
   }
   if (!ts_fifo_add(work_queue, sp)) {
      fprintf(stderr, "Error adding `%s' to linked list\n", sp);
      free(sp);
      status = 0;
      goto cleanup;
   }
   if (len == 1 && d[0] == '/')
      d[0] = '\0';
   /*
    * read entries from the directory
    */
   while (status && (dent = readdir(dd)) != NULL) {
      if (strcmp(".", dent->d_name) == 0 || strcmp("..", dent->d_name) == 0)
         continue;
      if (dent->d_type & DT_DIR) {
         char b[4096];
         sprintf(b, "%s/%s", d, dent->d_name);
	 status = processDirectory(b, ll, 0);
      }
   }
cleanup:
   (void) closedir(dd);
   return status;
}

/*
 * routine to convert bash pattern to regex pattern
 * 
 * e.g. if bashpat is "*.c", pattern generated is "^.*\.c$"
 *      if bashpat is "a.*", pattern generated is "^a\..*$"
 *
 * i.e. '*' is converted to ".*"
 *      '.' is converted to "\."
 *      '?' is converted to "."
 *      '^' is put at the beginning of the regex pattern
 *      '$' is put at the end of the regex pattern
 *
 * assumes 'pattern' is large enough to hold the regular expression
 * By Joe Sventek.
 */
static void cvtPattern(char pattern[], const char *bashpat) {
   char *p = pattern;

   *p++ = '^';
   while (*bashpat != '\0') {
      switch (*bashpat) {
      case '*':
         *p++ = '.';
	 *p++ = '*';
	 break;
      case '.':
         *p++ = '\\';
	 *p++ = '.';
	 break;
      case '?':
         *p++ = '.';
	 break;
      default:
         *p++ = *bashpat;
      }
      bashpat++;
   }
   *p++ = '$';
   *p = '\0';
}
