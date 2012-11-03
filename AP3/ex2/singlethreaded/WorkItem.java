package singlethreaded;


public class WorkItem {
	private boolean killswitch;
	private String path;

	public WorkItem(String path) {
		killswitch = false;
		this.path = path;
	}

	public WorkItem(boolean killswitch){
		this.killswitch = killswitch;
		this.path = "BOGUS ITEM";
	}

	/**
	* mechanism for killing workers:
	* they must shut down after consuming a killswitch item.
	* There will be exactly one killswitch item in the queue for each worker thread.
	*/
	public boolean isKillswitch(){
		return killswitch;
	}

	/**
	* The full relative path to the file, including the file name.
	*/
	public String getPath() {
		return path;
	}

	/**
	* The file name, not including the leading directory path (if any)
	*/
	public String getName() {
		return path.substring(path.lastIndexOf("/") + 1);
	}
}
