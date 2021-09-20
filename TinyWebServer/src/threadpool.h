/**
 * @brief Thread Pool
 * @author Ho 229
 * @date 2021/9/10
 * @note Thread pool without std::promise
 */

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <queue>
#include <atomic>
#include <vector>
#include <memory>
#include <thread>
#include <functional>
#include <condition_variable>

class ThreadPool
{
public:
    explicit ThreadPool(size_t count = std::thread::hardware_concurrency());
    ~ThreadPool();

    template<class F, class... Args>
    void enqueue(F&& f, Args&&... args);

private:
    // need to keep track of threads so we can join them
    std::vector<std::thread> m_workers;
    // the task queue
    std::queue<std::function<void()>> m_tasks;

    // synchronization
    std::mutex m_mutex;
    std::atomic_bool m_stop = false;
    std::condition_variable m_condition;
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads)
{
    for(size_t i = 0; i < threads; ++i)
        m_workers.emplace_back([this] {
            for(;;)
            {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(this->m_mutex);

                    this->m_condition.wait(lock,
                        [this]{ return this->m_stop || !this->m_tasks.empty(); });

                    if(this->m_stop && this->m_tasks.empty())
                        return;

                    task = std::move(this->m_tasks.front());
                    this->m_tasks.pop();
                }

                task();
            }
        });
}

// add new work item to the pool
template<class F, class... Args>
void ThreadPool::enqueue(F&& f, Args&&... args)
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::function<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    {
        std::unique_lock<std::mutex> lock(m_mutex);

        // don't allow enqueueing after stopping the pool
        if(m_stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        m_tasks.emplace([task](){ (*task)(); });
    }

    m_condition.notify_one();
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    m_stop = true;

    m_condition.notify_all();

    for(std::thread &worker: m_workers)
        worker.join();
}

#endif // THREADPOOL_H
