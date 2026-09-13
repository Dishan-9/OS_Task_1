# OS_Task_1
1.producer consumer
    I built this Java program to solve the classic Producer-Consumer synchronization challenge by using counting semaphores (Semaphore) instead of manual wait() and notify() locks. I created a shared buffer with two semaphores—giving my producer 1 permit so it starts immediately and my consumer 0 permits so it has to wait. Whenever my producer generates a number, it hands over the baton by releasing a consumer permit; my consumer then picks it up, prints the value, and hands control right back by releasing the producer permit. By coordinating the threads this way, I achieved a perfectly alternating ping-pong flow without race conditions, eliminated CPU-wasting busy waits, and relied on Java's built-in memory guarantees to make sure my data passes between threads safely and cleanly.
logic:PRODUCER ↓ Creates data ↓ BUFFER ↓ Stores data temporarily ↓ CONSUMER ↓ Uses data

2.matrix multiplication
    d
