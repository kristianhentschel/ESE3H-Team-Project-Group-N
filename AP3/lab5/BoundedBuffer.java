public class BoundedBuffer<T> {

   private int count;
   private int in;
   private int out;
   private int size;
   private T[] buffer;

   @SuppressWarnings(("unchecked"))
   public BoundedBuffer(int N) {
      count = 0;
      in = 0;
      out = 0;
      size = N;
      buffer = (T[]) new Object[N];
   }

   public BoundedBuffer() {
      this(10);
   }

   public synchronized void put(T t) {
      while (count == size)
         try {
            wait();
	 } catch (Exception e) {};
      buffer[in] = t;
      in = (in + 1) % size;
      count++;
      notifyAll();
   }

   public synchronized T get() throws InterruptedException {
      while (count == 0)
         wait();
      T t = buffer[out];
      out = (out + 1) % size;
      count--;
      notifyAll();
      return t;
   }
}
