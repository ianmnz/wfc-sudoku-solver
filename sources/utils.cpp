#include "utils.hpp"


namespace utils
{

/**
 * @brief Sample one element of array
 *
 * @param array Array of values
 * @return Chosen value
 */
int sample(const std::vector<int>& array)
{
    std::vector<int> chosen;
    std::sample(array.cbegin(), array.cend(), std::back_inserter(chosen), 1, g);
    return chosen[0];
}

/**
 * @brief Shuffle array
 *
 * @param array Reference to an array
 */
void shuffle(std::vector<int>& array)
{
    std::shuffle(array.begin(), array.end(), g);
}


thread_pool::thread_pool(const int nb_threads)
{
    _threads.reserve(nb_threads);

    for (int i = 0; i < nb_threads; ++i) {
        _threads.emplace_back([this] {
            while (true) {
                Task task;
                {
                    // Pop from tasks
                    std::unique_lock<std::mutex> lock(_mtx);
                    _cv.wait(lock, [this]() {
                        return !_tasks.empty() || _stop_pool;
                    });

                    if (_stop_pool && _tasks.empty()) {
                        return;
                    }

                    task = std::move(_tasks.front());
                    _tasks.pop();
                }
                task();
            }
        });
    }
}

thread_pool::~thread_pool()
{
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _stop_pool = true;
    }

    _cv.notify_all();

    for (auto& th : _threads) {
        th.join();
    }
}

void thread_pool::enqueue(Task task)
{
    {
        std::lock_guard<std::mutex> lock(_mtx);
        _tasks.emplace(std::move(task));
    }
    _cv.notify_one();
}

}  // namespace utils
