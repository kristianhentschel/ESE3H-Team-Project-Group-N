import java.util.concurrent.*;
import java.io.*;
import java.util.regex.*;

public class FileMatcher implements Runnable {
	private BlockingQueue<String> work_queue;
	private BlockingQueue<String> matches;
	private final Pattern pattern;

	public FileMatcher (BlockingQueue<String> work_queue, BlockingQueue<String> matches, Pattern pattern) {
		this.work_queue = work_queue;
		this.matches = matches;
		this.pattern = pattern;
	}

	public void run() {
		String dir;
		try {
			//while the item taken from the queue is not an empty string:
			while( !(dir = work_queue.take()).equals("")) {
				File dirfd = new File(dir); // create a File object
				String entries[] = dirfd.list();

				for (String entry : entries ) {
					//check if entry is itself a dir and skip if it is
					File entryfd = new File(dir + "/" + entry);
					if (entryfd.isDirectory())
						continue;
						
					// apply regex pattern
					Matcher m = pattern.matcher(entry);
					if (m.matches()){
						matches.put(dir + "/" + entry);
					}
				}
			}
		} catch (InterruptedException e) {
			System.err.printf("%s was interrupted\n", Thread.currentThread().getName());
			return;
		}	
	}

}
