#define _BSD_SOURCE 1

#include <stdio.h>
#include <dirent.h>
#include <regex.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "ts_fifo.h"
#include "re.h"

#define CRAWLER_THREADS 1

/* recursive method that adds directories to the work queue. */
static int processDirectory(char *dirname, ts_fifo work_queue, int verbose);

/* convert given bash pattern to regular expression */
static void cvtPattern(char pattern[], const char *bashpat); 

static int applyRe(char *dir, RegExp *reg, ts_fifo ts); 


/* the worker thread's main method */
struct worker_args {
	ts_fifo work_queue;
	ts_fifo results; 
	RegExp *reg;
};

static void *worker(void *args);

int main(int argc, char *argv[]) {
	//initialise variables
	int i;
	pthread_t threads[CRAWLER_THREADS];
	struct worker_args *args;
	char pattern[1024];
	RegExp *reg;
	char *dir;

	//START parse arguments (taken from Joe's starting code)
	if (argc < 2) {
	  fprintf(stderr, "Usage: ./fileCrawler pattern [dir] ...\n");
	  return -1;
	}
	/*
	* convert bash expression to regular expression and compile
	*/
	cvtPattern(pattern, argv[1]);
	if ((reg = re_create()) == NULL) {
	  fprintf(stderr, "Error creating Regular Expression Instance\n");
	  return -1;
	}
	if (! re_compile(reg, pattern)) {
	  char eb[4096];
	  re_status(reg, eb, sizeof eb);
	  fprintf(stderr, "Compile error - pattern: `%s', error message: `%s'\n",
			  pattern, eb);
	  re_destroy(reg);
	  return -1;
	}
	//END Joe's code

	//setup shared data structures
	ts_fifo work_queue = ts_fifo_create();
	ts_fifo results = ts_fifo_create();

	//create and launch worker threads
	if( (args = malloc(sizeof(struct worker_args))) == NULL)
		return -1;
	
	args->work_queue = work_queue;
	args->results = results;
	args->reg = reg;
	
	for (i = 0; i < CRAWLER_THREADS; i++) {
		if(pthread_create(&threads[i], NULL, worker, (void *) args))
			fprintf(stderr, "could not launch thread %d\n", i);
	}

	//fill work_queue with actual data
	
	//add a suicide command for each thread so they will die when no more work is left.

	//wait for all threads to die
	for (i = 0; i < CRAWLER_THREADS; i++){	
		pthread_join(threads[i], NULL);
	}

	//harvest and print results
	while( (dir = (char *) ts_fifo_dequeue(results)) != NULL ) {
		printf("%s\n", dir);
		free(dir); //TODO
	}


	//destroy shared data structures
	ts_fifo_destroy(work_queue);
	ts_fifo_destroy(results);

	free(args);

	return 0;
}



static void *worker(void *args_voidstar) {
	char *dir;
	struct worker_args *args = (struct worker_args *) args_voidstar; 
	while( *(dir = (char *) ts_fifo_dequeue(args->work_queue)) != '\0' ){
		applyRe(dir, args->reg, args->results);
	}

	fprintf(stderr, "Worker: My work here is done.\n");

	return NULL;
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

static int processDirectory(char *dirname, ts_fifo work_queue, int verbose) {
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
   if (!ts_fifo_enqueue(work_queue, (void *) sp)) {
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
	 status = processDirectory(b, work_queue, 0);
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


/*
 * applies regular expression pattern to contents of the directory
 *
 * for entries that match, the fully qualified pathname is inserted into
 * the treeset
 * By Joe Sventek. modified to add to queue instead of tree set.
 */
static int applyRe(char *dir, RegExp *reg, ts_fifo ts) {
   DIR *dd;
   struct dirent *dent;
   int status = 1;

   /*
    * open the directory
    */
   if ((dd = opendir(dir)) == NULL) {
      fprintf(stderr, "Error opening directory `%s'\n", dir);
      return 0;
   }
   /*
    * for each entry in the directory
    */
   while (status && (dent = readdir(dd)) != NULL) {
      if (strcmp(".", dent->d_name) == 0 || strcmp("..", dent->d_name) == 0)
         continue;
      if (!(dent->d_type & DT_DIR)) {
         char b[4096], *sp;
	 /*
	  * see if filename matches regular expression
	  */
	 if (! re_match(reg, dent->d_name))
            continue;
         sprintf(b, "%s/%s", dir, dent->d_name);
	 /*
	  * duplicate fully qualified pathname for insertion into queue
	  */
	 if ((sp = strdup(b)) != NULL) {
            if (!ts_fifo_enqueue(ts, sp)) {
               fprintf(stderr, "Error adding `%s' to queue\n", sp);
	       free(sp);
	       status = 0;
	       break;
	    }
	 } else {
            fprintf(stderr, "Error adding `%s' to queue\n", b);
	    status = 0;
	    break;
	 }
      }
   }
   (void) closedir(dd);
   return status;
}
