public class Composite {
	private String s;
	private BoundedBuffer<String> bb;


	public Composite(String s, BoundedBuffer<String> bb) {
		this.s = s;
		this.bb = bb;
	}

	public String getString() {
		return s;
	}

	public BoundedBuffer<String> getBB() {
		return bb;
	}
}
