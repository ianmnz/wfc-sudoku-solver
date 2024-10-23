#include "utils.hpp"


namespace utils
{

std::vector<int> sample(const std::vector<int>& array, const int nb_elem)
{
    std::vector<int> chosen;
    chosen.reserve(nb_elem);

    std::sample(array.cbegin(), array.cend(), std::back_inserter(chosen), nb_elem, g);

    return chosen;
}

void shuffle(std::vector<int>& array)
{
    std::shuffle(array.begin(), array.end(), g);
}


thread_pool::thread_pool(const int nb_threads)
{
    _threads.reserve(nb_threads);

    for (int i = 0; i < nb_threads; ++i) {
        _threads.emplace_back([this]{
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

    for(auto& th: _threads) {
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

}   // namespace utils
