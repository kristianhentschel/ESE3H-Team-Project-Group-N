package singlethreaded;

import java.util.concurrent.*;
import java.io.*;
import java.util.regex.*;
public class FileCrawler {

	private static BlockingQueue<WorkItem> work_queue;

	public static void main(String[] args) throws InterruptedException {
		//parse args
		String start_dir = ".";
		String search_string = "*.java";

		//translate search string to Regex pattern
		Pattern search_pattern = Pattern.compile(Regex.cvtPattern(search_string));
		
		//fill work queue recursively with directories
		work_queue = new LinkedBlockingQueue<WorkItem>();
		processDirectory(start_dir);

		//place suicide commands in queue for each thread
		work_queue.add( new WorkItem(true) );

		//remove each item from work queue, match it against search pattern, and place path in matches if it matches.
		BlockingQueue<WorkItem> matches = new LinkedBlockingQueue<WorkItem>();
		WorkItem item;
		while( !(item = work_queue.take()).isKillswitch() ) {
			
			// create a matcher against that line of input
			Matcher m = search_pattern.matcher(item.getName());
			if (m.matches())
				matches.add(item);
		}

		//print matches -- need to sort this?
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
				work_queue.put(new WorkItem(name)); // add file item to work queue
			}
		} catch (Exception e) {
			System.err.println("Error processing "+name+": "+e);
		}
	}

}
