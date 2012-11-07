public class Consumer implements Runnable {

   private String name;
   private BoundedBuffer<String> bb;

   public Consumer(int i, BoundedBuffer<String> bb) {
      this.name = "Consumer"+i;
      this.bb = bb;
   }

   public void run() {
      while (true) {
         String s;
         try {
            s = bb.get();
         } catch (InterruptedException e) { return; }
         System.out.println(name+" received "+s);
      }
   }
}
