package ProducerConsumerProblem;
import java.util.concurrent.Semaphore;

class Semabuff {
    private int val;
    private final Semaphore producePermit = new Semaphore(1);
    private final Semaphore consumePermit = new Semaphore(0);

    public void produce(int val) {
        try {
            producePermit.acquire();
            this.val = val;
            System.out.println("PRODUCED : " + val);
            consumePermit.release();
        } catch (InterruptedException e){
            Thread.currentThread().interrupt();
        }
    }

    public void consume() {
        try {
            consumePermit.acquire();
            System.out.println("CONSUMED :" + val);
            producePermit.release();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }
}

public class ProducerConsumerSemaphore {
    public static void main(String[] args) {
        Semabuff buffer = new Semabuff();

        Thread producerThread = new Thread(new Runnable() {
            @Override
            public void run() {
                for (int i = 1; i <= 5; i++) {
                    buffer.produce(i);
                }
            }
        });

        Thread consumerThread = new Thread(() -> {
            for (int i = 1; i <= 5; i++) {
                buffer.consume();
            }
        });
        producerThread.start();
        consumerThread.start();
    }
}