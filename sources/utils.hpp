#pragma once

#include <algorithm>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <random>
#include <thread>
#include <vector>

#define N 9
#define BOX 3

namespace utils
{

inline std::array<int, 2> array2grid(const int index) { return {(index / N), (index % N)}; }
inline int grid2array(const int i, const int j) { return i * N + j; }

#ifdef DEBUG
inline std::seed_seq seed{2};
inline std::mt19937 g(seed);
#else
inline std::random_device rd;
inline std::mt19937 g(rd());
#endif

int sample(const std::vector<int>& array);
void shuffle(std::vector<int>& array);

using Task = std::function<void()>;

class thread_pool
{
public:
    thread_pool(const int nb_threads);
    ~thread_pool();
    void enqueue(Task task);

private:
    std::vector<std::thread> _threads;
    std::queue<Task> _tasks;
    std::condition_variable _cv;
    std::mutex _mtx;

    bool _stop_pool = false;
};

}  // namespace utils
