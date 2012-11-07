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
				//Blocking Queue includes includes wait() and notifyAll()?
				Matcher m = pattern.matcher(item.getName());
				if (m.matches())
					matches.add(item);
			}
			System.err.println("Got killswitch item");
		} catch (InterruptedException e) {
			System.err.println("Thread interrupted");
			return;
		}	
	}

}
