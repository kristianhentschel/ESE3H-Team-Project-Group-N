import java.util.concurrent.*;
import java.io.*;
import java.util.regex.*;

public class FileCrawler {
	private static BlockingQueue<String> work_queue;
	private static PriorityBlockingQueue<String> matches;
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
		work_queue = new LinkedBlockingQueue<String>();
		
		//initialise results data structure.
		matches = new PriorityBlockingQueue<String>();
		
		//initialise and start worker threads, they'll wait for items to be added to the queue
		Thread workers[] = new Thread[crawler_threads];

		for( int i = 0; i < crawler_threads; i++ ) {
			workers[i] = new Thread(new FileMatcher(work_queue, matches, search_pattern));
			workers[i].start();
		}
		
		System.err.printf("=== Started %d worker threads.\n", crawler_threads);
		
		//fill work queue with directories (recursive)
		if (args.length == 1) {
			processDirectory(".");
		} else {
			for(int i = 1; i < args.length; i++ )
			{
				processDirectory(args[i]);
			}
		}
		
		//place one suicide command in queue for each thread
		for( int i = 0; i < crawler_threads; i++ ) {
			work_queue.add( "" );
		}

		System.err.println("=== filled work queue, waiting for workers to finish.");

		for( int i = 0; i < crawler_threads; i++ ) {
			workers[i].join();			
		}

		System.err.println("=== all worker threads have finished - go to harvesting");

		//print matches -- need to sort this?
		String m;
		while( ( m = matches.poll() ) != null ) {
			System.out.println(m);
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
		//remove trailing / if existing and name != /.
		if (name.charAt(name.length()-1) == '/' && name.length() > 1) {
			name = name.substring(0, name.length()-1);
		}
		try {
			File file = new File(name);	// create a File object
			if (file.isDirectory()) {	// a directory - could be symlink
				String entries[] = file.list();
				if (entries != null) {	// not a symlink
					// only add directories to work queue
					work_queue.put(name);
					// process directory child entries
					for (String entry : entries ) {
						if (entry.compareTo(".") == 0)
							continue;
						if (entry.compareTo("..") == 0)
							continue;
						// Don't need to check for directory/file here.
						processDirectory(name+"/"+entry);
					}
				}
			} 
		} catch (Exception e) {
			System.err.println("Error processing "+name+": "+e);
		}
	}

}
