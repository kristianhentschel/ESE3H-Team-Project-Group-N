public class ProducerConsumer {

   private static void usage() {
       System.err.println("usage: java -classpath . ProducerConsumer [-p<n>] [-c<n>] [-m<n>]");
       System.exit(1);
   }

   public static void main(String Args[]) {
      int nproducers, nconsumers, nmessages;
// initialize number of producers, consumers, messages from each producer
      nproducers = 1;
      nconsumers = 1;
      nmessages = 100;

// process arguments, providing reminder if any are invalid
      int n = Args.length;
      for (int i = 0; i < n; i++) {
         String s = Args[i].toLowerCase();
         if (s.startsWith("-p")) {
            nproducers = (new Integer(Args[i].substring(2))).intValue();
	 } else if (s.startsWith("-c")) {
            nconsumers = (new Integer(Args[i].substring(2))).intValue();
	 } else if (s.startsWith("-m")) {
            nmessages = (new Integer(Args[i].substring(2))).intValue();
	 } else
            ProducerConsumer.usage();
      }

// create BoundedBuffer through which the producers and consumers synchronize
      BoundedBuffer<String> bb = new BoundedBuffer<String>(10);

// create the producer and consumer threads
      Thread[] prods = new Thread[nproducers];
      Thread[] conss = new Thread[nconsumers];
      for (int i = 0; i < nproducers; i++)
         prods[i] = new Thread(new Producer(i, bb, nmessages));
      for (int i = 0; i < nconsumers; i++)
         conss[i] = new Thread(new Consumer(i, bb));

// start the consumers first, then the producers
      for (int i = 0; i < nconsumers; i++)
         conss[i].start();
      for (int i = 0; i < nproducers; i++)
         prods[i].start();

// now wait for the producers to finish
      for (int i = 0; i < nproducers; i++)
         try {
            prods[i].join();
	 } catch (Exception e) {};

// now interrupt all consumers and wait for them to finish
      for (int i = 0; i < nconsumers; i++) {
         conss[i].interrupt();
         try {
            conss[i].join();
	 } catch (Exception e) {};
      }

// now exit gracefully
   }
}
