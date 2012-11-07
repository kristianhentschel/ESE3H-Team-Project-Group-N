public class Producer implements Runnable {

   private String name;
   private BoundedBuffer<String> bb;
   private int nmsgs;

   public Producer(int i, BoundedBuffer<String> bb, int nmsgs) {
      this.name = "Producer"+i;
      this.bb = bb;
      this.nmsgs = nmsgs;
   }

   public void run() {
      for (int i = 0; i < nmsgs; i++) {
         String msg = new String(name+": message "+i);
         bb.put(msg);
      }
   }
}
