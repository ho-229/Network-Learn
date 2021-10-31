/**
 * @author Ho 229
 * @date 2021/8/20
 */

#ifndef TIMERMANAGER_H
#define TIMERMANAGER_H

#include <mutex>
#include <queue>
#include <atomic>
#include <chrono>
#include <memory>

namespace Time = std::chrono;

class AbstractSocket;

template <typename T>
class Timer
{
public:
    Timer(const T& data) :
        m_userData(data),
        m_active(Time::system_clock::now())
    {}

    inline void deleteLater() { m_isDisable = true; }

    inline bool isDisable() const { return m_isDisable; }

    inline auto duration() const
    {
        return Time::duration_cast<Time::milliseconds>(
                   Time::system_clock::now() - m_active).count();
    }

    inline const T userData() const { return m_userData; }

private:
    std::atomic_bool m_isDisable = false;

    const T m_userData;
    const Time::system_clock::time_point m_active;
};

template <typename T>
using TimerItem = std::shared_ptr<Timer<T>>;

template <typename T>
class TimerManager
{
public:
    explicit TimerManager() {}

    void setTimeout(int ms) { m_timeout = ms > 0 ? ms : 10000; }
    int timeout() const { return m_timeout; }

    Timer<T>* addTimer(const T& data)
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        auto newTimer = std::make_shared<Timer<T>>(data);
        m_queue.push(newTimer);
        return newTimer.get();
    }

    /**
     * @return true if it's timeout
     */
    bool checkTop(T &userData)
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        while(true)
        {
            if(m_queue.empty())
                return false;
            else if(m_queue.front()->isDisable())
                m_queue.pop();
            else
                break;
        }

        const auto top = m_queue.front();

        if(top->duration() >= m_timeout)
        {
            userData = top->userData();
            m_queue.pop();
            return true;
        }

        return false;
    }

private:
    int m_timeout = 30000;      // 30s

    std::mutex m_mutex;

    std::queue<TimerItem<T>, std::deque<TimerItem<T>>> m_queue;
};

#endif // TIMERMANAGER_H
