#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>
#include <random>
#include <fstream>

constexpr int N = 100;
constexpr int K = 100;
constexpr int M = 100;

// Thread-safe log recorder
struct FinishedCell {
    int row;
    int col;
    long long value;
};

class ExecutionRecorder {
public:
    void record(int r, int c, long long val) {
        std::lock_guard<std::mutex> lock(recordMutex);
        log.push_back({r, c, val});
    }

    void exportToCSV(const std::string& filename) {
        std::ofstream file(filename);
        file << "row,col,val\n";
        for (const auto& item : log) {
            file << item.row << "," << item.col << "," << item.value << "\n";
        }
    }

private:
    std::mutex recordMutex;
    std::vector<FinishedCell> log;
};

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads) : stop(false), activeTasks(0) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        this->cv.wait(lock, [this]() {
                            return this->stop || !this->tasks.empty();
                        });

                        if (this->stop && this->tasks.empty()) return;

                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    task();

                    {
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        --activeTasks;
                        if (activeTasks == 0 && tasks.empty()) {
                            finishedCv.notify_all();
                        }
                    }
                }
            });
        }
    }

    void enqueue(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.push(std::move(task));
            ++activeTasks;
        }
        cv.notify_one();
    }

    void waitUntilDone() {
        std::unique_lock<std::mutex> lock(queueMutex);
        finishedCv.wait(lock, [this]() {
            return tasks.empty() && activeTasks == 0;
        });
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        cv.notify_all();
        for (std::thread& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::condition_variable finishedCv;
    bool stop;
    size_t activeTasks;
};

void computeSingleCell(const std::vector<std::vector<long long>>& A,
                       const std::vector<std::vector<long long>>& B,
                       std::vector<std::vector<long long>>& C,
                       ExecutionRecorder& recorder,
                       int row, int col) {
    long long sum = 0;
    for (int k = 0; k < K; ++k) {
        sum += A[row][k] * B[k][col];
    }
    C[row][col] = sum;
    recorder.record(row, col, sum);
}

int main() {
    std::vector<std::vector<long long>> A(N, std::vector<long long>(K));
    std::vector<std::vector<long long>> B(K, std::vector<long long>(M));
    std::vector<std::vector<long long>> C(N, std::vector<long long>(M, 0));

    std::mt19937 rng(42);
    std::uniform_int_distribution<long long> dist(0, 9);

    for (int i = 0; i < N; ++i)
        for (int j = 0; j < K; ++j)
            A[i][j] = dist(rng);

    for (int i = 0; i < K; ++i)
        for (int j = 0; j < M; ++j)
            B[i][j] = dist(rng);

    unsigned int coreCount = std::thread::hardware_concurrency();
    if (coreCount == 0) coreCount = 4;

    std::cout << "Submitting " << (N * M) 
              << " independent cell operations across " 
              << coreCount << " hardware threads...\n";

    ThreadPool pool(coreCount);
    ExecutionRecorder recorder;

    auto startTime = std::chrono::high_resolution_clock::now();

    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < M; ++c) {
            pool.enqueue([&A, &B, &C, &recorder, r, c]() {
                computeSingleCell(A, B, C, recorder, r, c);
            });
        }
    }

    pool.waitUntilDone();

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = endTime - startTime;

    std::cout << "Finished all cell calculations in: " << elapsed.count() << " ms\n";
    
    recorder.exportToCSV("execution_log.csv");
    std::cout << "Saved real thread execution order to 'execution_log.csv'.\n";

    return 0;
}