import java.util.Scanner;

public class bbTest {

   public static class Cons implements Runnable {
      private BoundedBuffer<String> theBuffer;

      public Cons(BoundedBuffer<String> theBuffer) {
         this.theBuffer = theBuffer;
      }

      public void run() {
         while (true) {
            try {
               String s = theBuffer.get();
	       System.out.println(s);
	    } catch (InterruptedException e) { return; }
	 }
      }
   }

   public static void main(String Args[]) {
      BoundedBuffer<String> theBuffer = new BoundedBuffer<String>();
      Cons c = new Cons(theBuffer);
      Thread t = new Thread(c);
      Scanner stdin = new Scanner(System.in);

      t.start();
      while (stdin.hasNext()) {
         String s = stdin.nextLine();
	 theBuffer.put(s);
      }
      t.interrupt();
      try {
         t.join();
      } catch (Exception e) {}
      System.exit(0);
   }
}
