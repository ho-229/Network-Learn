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
#include <vector>
#include <thread>
#include <functional>
#include <condition_variable>

typedef std::function<void()> TaskHandler;

class ThreadPool
{
public:
    explicit ThreadPool(size_t count = std::thread::hardware_concurrency())
    {
        if(count < 1)
            count = std::thread::hardware_concurrency();

        for(size_t i = 0; i < count; ++i)
        {
            m_workers.emplace_back(([this] {
                while(m_runnable)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(m_mutex);
                        if(m_tasks.empty())
                            m_condition.wait(lock);

                        task = std::move(m_tasks.front());
                        m_tasks.pop();
                    }

                    task();
                }
            }));
        }
    }

    ~ThreadPool()
    {
        m_runnable = false;

        for(auto& thread : m_workers)
            if(thread.joinable())
                thread.join();
    }

    void start(const TaskHandler& func)
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_tasks.push(func);
        }

        m_condition.notify_one();
    }

    size_t threadCount() const { return m_workers.size(); }

private:
    // need to keep track of threads so we can join them
    std::vector<std::thread> m_workers;

    // the task queue
    std::queue<TaskHandler> m_tasks;

    // synchronization
    std::mutex m_mutex;
    std::condition_variable m_condition;

    bool m_runnable = true;
};


#endif // THREADPOOL_H
