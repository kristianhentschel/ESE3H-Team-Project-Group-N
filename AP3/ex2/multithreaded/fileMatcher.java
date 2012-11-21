/**
 * Class fileMatcher: takes directories from the work queue,
 * and scans the files in them, adding to the matches .
 * This is my own work as defined in the Ethics Agreement I have signed,
 * except for the code sections marked as taken from the sample code
 * given in the exercise handout.
 * @author Kristian Hentschel, 1003734h, for AP3 Exercise 2.
 */

import java.util.concurrent.*;
import java.io.*;
import java.util.regex.*;

public class fileMatcher implements Runnable {
	private BlockingQueue<String> work_queue;
	private BlockingQueue<String> matches;
	private final Pattern pattern;

	public fileMatcher (BlockingQueue<String> work_queue, BlockingQueue<String> matches, Pattern pattern) {
		this.work_queue = work_queue;
		this.matches = matches;
		this.pattern = pattern;
	}

	public void run() {
		String item;

		try {
			while( !(item = work_queue.take()).equals("")) {
				File file = new File(item);
				String entries[] = file.list();
				// process directory child entries
				for (String entry : entries ) {
					//check if entry is itself a dir and skip if it is
					File e = new File(item + "/" + entry);
					if (e.isDirectory())
						continue;
						
					// apply regex pattern
					Matcher m = pattern.matcher(entry);
					if (m.matches()){
						matches.put(item + "/" + entry);
					}
				}
			}
		} catch (InterruptedException e) {
			System.err.printf("%s was interrupted\n", Thread.currentThread().getName());
			return;
		}	
	}

}
