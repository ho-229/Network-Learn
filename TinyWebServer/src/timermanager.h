/**
 * @author Ho 229
 * @date 2021/8/20
 */

#ifndef TIMERMANAGER_H
#define TIMERMANAGER_H

#include <queue>
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

    bool isDisable = false;

    auto duration() const
    {
        return Time::duration_cast<Time::milliseconds>(
                   Time::system_clock::now() - m_active).count();
    }

    const T userData() const { return m_userData; }

private:
    const T m_userData;
    const Time::system_clock::time_point m_active;
};

template <typename T>
using TimerItem = std::shared_ptr<Timer<T>>;

template <typename T>
struct TimerCompare
{
    bool operator()(const TimerItem<T>& a, const TimerItem<T>& b) const
    { return a->duration() > b->duration(); }
};

template <typename T>
class TimerManager
{
public:
    explicit TimerManager() {}

    void setTimeout(int ms) { m_timeout = ms > 0 ? ms : 10000; }
    int timeout() const { return m_timeout; }

    Timer<T>* addTimer(const T& data)
    {
        auto newTimer = new Timer<T>(data);
        m_queue.push(TimerItem<T>(newTimer));
        return newTimer;
    }

    /**
     * @return true if it's timeout
     */
    bool checkTop(T &userData)
    {
        while(true)
        {
            if(m_queue.empty())
                return false;
            else if(m_queue.top()->isDisable)
                m_queue.pop();
            else
                break;
        }

        const auto top = m_queue.top();
        const bool ret = top->duration() >= m_timeout;

        if(ret)
            userData = top->userData();

        return ret;
    }

private:
    int m_timeout = 30000;      // 30s

    std::priority_queue<TimerItem<T>,
                        std::deque<TimerItem<T>>,
                        TimerCompare<T>>
        m_queue;
};

#endif // TIMERMANAGER_H
