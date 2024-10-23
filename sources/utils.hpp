#pragma once

#include <random>
#include <vector>
#include <algorithm>
#include <thread>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <functional>


namespace utils
{

inline std::random_device rd;
inline std::mt19937 g(rd());

// inline std::seed_seq seed{2};
// inline std::mt19937 g(seed);

std::vector<int> sample(const std::vector<int>& array, const int nb_elem = 1);
void shuffle(std::vector<int>& array);

using Task = std::function<void ()>;

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

} // namespace utils
