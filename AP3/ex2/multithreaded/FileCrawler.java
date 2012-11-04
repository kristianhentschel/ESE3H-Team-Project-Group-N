import java.util.concurrent.*;
import java.io.*;
import java.util.regex.*;

public class FileCrawler {
	private static BlockingQueue<WorkItem> work_queue, matches;
	private static String start_dir;
	private static String search_string;
	private static Pattern search_pattern;
	private static int crawler_threads;


	public static void main(String[] args) throws InterruptedException {
		//parse arguments	
		if (args.length == 0) {
			System.err.println("Usage: fileCrawler pattern [directory]");
			System.exit(1);
		}

		search_string = args[0];

		if (args.length >= 2)
			start_dir = args[1];
		else
			start_dir = "."; //default: start in working directory

		String envvar;
		if ((envvar = System.getenv("CRAWLER_THREADS")) == null) {
			crawler_threads = 2;
		} else {
			try {
				crawler_threads = Integer.decode(envvar);
			} catch (NumberFormatException e) {
				System.err.println("Invalid value in CRAWLER_THREADS, must be an integer.");
				System.exit(1);
			}
			if (crawler_threads < 1) {
				System.err.println("CRAWLER_THREADS must be greater than zero.");
				System.exit(1);
			}
		}
		//translate search string to Regex pattern
		Pattern search_pattern = Pattern.compile(Regex.cvtPattern(search_string));
		
		//initialise work queue
		work_queue = new LinkedBlockingQueue<WorkItem>();
		
		//initialise results data structure.
		matches = new LinkedBlockingQueue<WorkItem>();
		
		//initialise and start worker threads, they'll wait for items to be added to the queue
		Thread workers[] = new Thread[crawler_threads];

		for( int i = 0; i < crawler_threads; i++ ) {
			workers[i] = new Thread(new FileMatcher(work_queue, matches, search_pattern));
			workers[i].start();
		}
		
		//fill work queue with directories (recursive
		processDirectory(start_dir);

		//place one suicide command in queue for each thread
		for( int i = 0; i < crawler_threads; i++ ) {
			work_queue.add( new WorkItem(true) );
		}

		System.err.println("=== filled work queue, waiting for workers to finish.");

		for( int i = 0; i < crawler_threads; i++ ) {
			workers[i].join();			
		}

		System.err.println("=== all worker threads have finished - go to harvesting");

		//print matches -- need to sort this?
		WorkItem item;
		while( ( item = matches.poll() ) != null ) {
			System.out.println(item.getPath());
		}
	}

	/**
	* Works on a single file system entry and
	* calls itself recursively if it turns out
	* to be a directory.
	* @author Taken from Joe's example code and modified to add item to work queue.
	* @param name    The name of a directory to visit
	*/
	public static void processDirectory( String name ) {
		try {
			File file = new File(name);	// create a File object
			if (file.isDirectory()) {	// a directory - could be symlink
				String entries[] = file.list();
				if (entries != null) {	// not a symlink
					for (String entry : entries ) {
						if (entry.compareTo(".") == 0)
							continue;
						if (entry.compareTo("..") == 0)
							continue;
						processDirectory(name+"/"+entry);
					}
				}
			} else { //not a directory
				//System.err.printf("Put:\t %s\n", name);
				work_queue.put(new WorkItem(name)); // add file item to work queue
			}
		} catch (Exception e) {
			System.err.println("Error processing "+name+": "+e);
		}
	}

}
