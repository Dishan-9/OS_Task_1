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

constexpr int ROWS_A = 100;
constexpr int COLS_A = 100; 
constexpr int COLS_B = 100;

struct CellResult {
    int row;
    int col;
    long long value;
};

class ExecutionLogger {
public:
    void record(int r, int c, long long val) {
        std::lock_guard<std::mutex> lock(mtx);
        records.push_back({r, c, val});
    }

    void dumpCsv(const std::string& filepath) {
        std::ofstream out(filepath);
        if (!out.is_open()) {
            std::cerr << "Failed to open " << filepath << " for writing\n";
            return;
        }

        out << "row,col,val\n";
        for (const auto& item : records) {
            out << item.row << "," << item.col << "," << item.value << "\n";
        }
    }

private:
    std::mutex mtx;
    std::vector<CellResult> records;
};

class ThreadPool {
public:
    explicit ThreadPool(size_t threads) {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(queueMtx);
                        cv.wait(lock, [this]() {
                            return stopping || !taskQueue.empty();
                        });

                        if (stopping && taskQueue.empty()) {
                            return;
                        }

                        task = std::move(taskQueue.front());
                        taskQueue.pop();
                    }

                    task();

                    {
                        std::lock_guard<std::mutex> lock(queueMtx);
                        --busyCount;
                        if (busyCount == 0 && taskQueue.empty()) {
                            doneCv.notify_all();
                        }
                    }
                }
            });
        }
    }

    void submit(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(queueMtx);
            taskQueue.push(std::move(task));
            ++busyCount;
        }
        cv.notify_one();
    }

    void waitAll() {
        std::unique_lock<std::mutex> lock(queueMtx);
        doneCv.wait(lock, [this]() {
            return taskQueue.empty() && busyCount == 0;
        });
    }

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(queueMtx);
            stopping = true;
        }
        cv.notify_all();

        for (auto& t : workers) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> taskQueue;
    std::mutex queueMtx;
    std::condition_variable cv;
    std::condition_variable doneCv;
    bool stopping = false;
    size_t busyCount = 0;
};

void multiplyCell(const std::vector<std::vector<long long>>& A,
                  const std::vector<std::vector<long long>>& B,
                  std::vector<std::vector<long long>>& C,
                  ExecutionLogger& logger,
                  int r, int c) {
    long long dot = 0;
    for (int k = 0; k < COLS_A; ++k) {
        dot += A[r][k] * B[k][c];
    }
    C[r][c] = dot;
    logger.record(r, c, dot);
}

int main() {
    using Matrix = std::vector<std::vector<long long>>;

    Matrix A(ROWS_A, std::vector<long long>(COLS_A));
    Matrix B(COLS_A, std::vector<long long>(COLS_B));
    Matrix C(ROWS_A, std::vector<long long>(COLS_B, 0));

    // Fill matrices with small random values
    std::mt19937 rng(1337);
    std::uniform_int_distribution<long long> dist(0, 9);

    for (int i = 0; i < ROWS_A; ++i) {
        for (int j = 0; j < COLS_A; ++j) {
            A[i][j] = dist(rng);
        }
    }

    for (int i = 0; i < COLS_A; ++i) {
        for (int j = 0; j < COLS_B; ++j) {
            B[i][j] = dist(rng);
        }
    }

    unsigned int workerCount = std::thread::hardware_concurrency();
    if (workerCount == 0) workerCount = 4;

    std::cout << "Dispatching " << (ROWS_A * COLS_B) << " tasks over " 
              << workerCount << " worker threads...\n";

    ThreadPool pool(workerCount);
    ExecutionLogger logger;

    auto t0 = std::chrono::steady_clock::now();

    for (int r = 0; r < ROWS_A; ++r) {
        for (int c = 0; c < COLS_B; ++c) {
            pool.submit([&A, &B, &C, &logger, r, c]() {
                multiplyCell(A, B, C, logger, r, c);
            });
        }
    }

    pool.waitAll();

    auto t1 = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> ms = t1 - t0;

    std::cout << "Done in " << ms.count() << " ms\n";

    logger.dumpCsv("execution_log.csv");
    std::cout << "Output saved to execution_log.csv\n";

    return 0;
}