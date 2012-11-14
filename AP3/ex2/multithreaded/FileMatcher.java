import java.util.concurrent.*;
import java.io.*;
import java.util.regex.*;

public class FileMatcher implements Runnable {
	private BlockingQueue<WorkItem> work_queue;
	private BlockingQueue<String> matches;
	private final Pattern pattern;

	public FileMatcher (BlockingQueue<WorkItem> work_queue, BlockingQueue<String> matches, Pattern pattern) {
		this.work_queue = work_queue;
		this.matches = matches;
		this.pattern = pattern;
	}

	public void run() {
		WorkItem item;

		try {
			while( !(item = work_queue.take()).isKillswitch()) {
				//Blocking Queue includes wait() and notifyAll()?
				//System.err.printf("%s took:\t %s\n", Thread.currentThread().getName(), item.getName());
				
				File file = new File(item.getPath());	// create a File object
				String entries[] = file.list();
				// process directory child entries
				for (String entry : entries ) {
					//check if entry is itself a dir and skip if it is
					File e = new File(item.getPath() + "/" + entry);
					if (e.isDirectory())
						continue;
						
					// apply regex pattern
					Matcher m = pattern.matcher(entry);
					if (m.matches()){
						matches.put(item.getPath() + "/" + entry);
					}
				}
			}
			//System.err.printf("%s died\n", Thread.currentThread().getName());
		} catch (InterruptedException e) {
			System.err.printf("%s was interrupted\n", Thread.currentThread().getName());
			return;
		}	
	}

}
