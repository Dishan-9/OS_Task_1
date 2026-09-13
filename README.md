# OS_Task_1
1.producer consumer:

I built this Java program to solve the classic Producer-Consumer synchronization challenge by using counting semaphores (Semaphore) instead of manual wait() and notify() locks. I created a shared buffer with two semaphores—giving my producer 1 permit so it starts immediately and my consumer 0 permits so it has to wait. Whenever my producer generates a number, it hands over the baton by releasing a consumer permit; my consumer then picks it up, prints the value, and hands control right back by releasing the producer permit. By coordinating the threads this way, I achieved a perfectly alternating ping-pong flow without race conditions, eliminated CPU-wasting busy waits, and relied on Java's built-in memory guarantees to make sure my data passes between threads safely and cleanly.

logic:PRODUCER ↓ Creates data ↓ BUFFER ↓ Stores data temporarily ↓ CONSUMER ↓ Uses data




2.matrix multiplication:

This program demonstrates concurrent computing and cache-conscious parallel architecture by performing a $100 \times 100$ matrix multiplication ($A \times B = C$) across $10,000$ independent cell operations. Rather than creating a separate operating system thread for every individual element—which would overwhelm kernel resources with excessive stack allocation and context switching—the application implements a custom, thread-safe ThreadPool. The pool caps concurrent OS worker threads to the host system's hardware execution capacity (std::thread::hardware_concurrency) and manages incoming cell tasks through a synchronized FIFO work queue guarded by a mutual exclusion lock (std::mutex) and condition variables (std::condition_variable). Each worker thread dequeues an individual cell computation task, calculates the vector dot product of the corresponding row in matrix $A$ and column in matrix $B$, writes the result directly into matrix $C$, and logs its precise completion coordinates into a thread-safe execution recorder. This logged completion sequence enables authentic visualization of non-deterministic thread scheduling, illustrating how asynchronous tasks interleave across available CPU cores while ensuring deterministic mathematical accuracy via scalar validation.

logic:The program splits 10,000 matrix cells into independent tasks and distributes them across a fixed set of CPU worker threads via a synchronized queue, writing each result and recording the exact order they finish for the animation.
