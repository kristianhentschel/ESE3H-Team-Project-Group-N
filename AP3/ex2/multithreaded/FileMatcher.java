import java.util.concurrent.*;
import java.io.*;
import java.util.regex.*;

public class FileMatcher implements Runnable {
	private BlockingQueue<WorkItem> work_queue, matches;
	private final Pattern pattern;

	public FileMatcher (BlockingQueue<WorkItem> work_queue, BlockingQueue<WorkItem> matches, Pattern pattern) {
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
				
				Matcher m = pattern.matcher(item.getName());
				if (m.matches()){
					matches.put(item);
				}
			}
			//System.err.printf("%s died\n", Thread.currentThread().getName());
		} catch (InterruptedException e) {
			System.err.printf("%s was interrupted\n", Thread.currentThread().getName());
			return;
		}	
	}

}
