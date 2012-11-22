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

/* tool function signatures (taken from Joe's code) */
static void cvtPattern(char pattern[], const char *bashpat); 

/* global variables */
static ts_fifo *work_queue;
static ts_fifo *results; 
static RegExp *reg;

/* worker thread and tool functions accessing shared data structures */
static void *worker(void *args);
static int process_directory(char *dirname);
static void process_file(char *filename);

/* main thread */
int main(int argc, char *argv[]) {
	//initialise variables
	int i;
	pthread_t threads[CRAWLER_THREADS];
	struct worker_args *args;
	char pattern[1024];
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
	//END Joe's argument parsing code


	//setup shared data structures in global variables
	work_queue = ts_fifo_create();
	results = ts_fifo_create();
	//TODO Joe uses a tree set for the results.
	//it should be a sorted data structure with O(log(n) insertion
	//maybe I will write an AVL tree or something.

	//create and launch worker threads
	for (i = 0; i < CRAWLER_THREADS; i++) {
		if(pthread_create(&threads[i], NULL, worker, (void *) args))
			fprintf(stderr, "could not launch thread %d\n", i);
	}

	//fill work_queue with initial data
	if(argc == 2) {
		ts_fifo_add(work_queue, strdup("."));
	} else {
		for(i = 2; i < argc; i++)
			ts_fifo_add(work_queue, argv[i]);
	}	
	
	//add a suicide command for each thread so they will die when no more work is left.
	//TODO can't really do this anymore as our workers are now producers
	//need to figure out a new way to signal that all dirs have been scanned.
	for (i = 0; i < CRAWLER_THREADS; i++){	
		ts_fifo_add(work_queue, "\0");
	}


	//wait for all threads to die
	for (i = 0; i < CRAWLER_THREADS; i++){	
		pthread_join(threads[i], NULL);
	}


	fprintf(stderr, "all workers finished, main thread harvesting now.\n");

	//harvest and print results
	//TODO this cannot work as this fifo queue blocks if it is empty.
	while( (dir = (char *) ts_fifo_remove(results)) != NULL ) {
		printf("%s\n", dir);
	}


	//destroy shared data structures
	ts_fifo_destroy(work_queue);
	ts_fifo_destroy(results);

	free(args);

	return 0;
}



static void *worker(void *args) {
	char *dir;
	while( *(dir = (char *) ts_fifo_remove(work_queue)) != '\0' ){
		process_directory(dir);
	}

	fprintf(stderr, "Worker: My work here is done.\n");

	return NULL;
}

/*
 * Non-recursive directory scanner and file matcher.
 * Opens a directory and scans its entries. Does not descend of subdirectories.
 * - If an entry is a file, it is matched against the regular expression and added to
 *   the results structure if it matches.
 * - If an entry is a directory, it is added to the work queue itself, to be picked up
 *   again later by any worker.
 * returns 1 on error, 0 on success.
 *
 * based on Joe's example code from processDirectory.
 */
static int process_directory(char *dirname){
	DIR *dd;
	struct dirent *dent;
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
		fprintf(stderr, "Error opening directory `%s'\n", d);
		return 1;
	}


	while (status && (dent = readdir(dd)) != NULL) {
		/* ignore . and .. entries */
		if (strcmp(".", dent->d_name) == 0 || strcmp("..", dent->d_name) == 0)
			continue;
		
		/* generate full path */
		char b[4096];
		char *p;
		sprintf(b, "%s/%s", d, dent->d_name);
		p = strdup(b);

		if (dent->d_type & DT_DIR) {
			/* sub directory, add to work queue */
			ts_fifo_add(work_queue, p);
		} else {
			/* normal file entry. match regular expression and add to results */
			process_file(p);
		}
	}

	(void) closedir(dd);
	return 0;
}

/* 
 * matches file against regular expression and adds it to the results data structure
 * if it matches. destroys given string afterwards to clean up.
 */
static void process_file(char *path){
	char *filename	= strrchr(path, '/') + 1; //assumes that string does not end with /.
	
	fprintf(stderr, "process_file() \t%s\n", path);
	
	if( re_match(reg, filename) ){
		ts_fifo_add(results, path);
	} else {
		free(path);
	}
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
