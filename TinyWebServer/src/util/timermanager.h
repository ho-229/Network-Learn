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

#define TIMER_THREAD_SAFE 0

template <typename T, typename TimeType>
class Timer
{
public:
    Timer(const TimeType& timeout, const T& data) :
        m_userData(data),
        m_deadline(std::chrono::system_clock::now() + timeout)
    {}

    inline void deleteLater() { m_isDisable = true; }

    inline bool isDisable() const { return m_isDisable; }

    inline const auto& deadline() const { return m_deadline; }

    inline const T userData() const { return m_userData; }

private:
    volatile bool m_isDisable = false;

    const T m_userData;
    const std::chrono::system_clock::time_point m_deadline;
};

template <typename T, typename TimeType>
using TimerItem = std::unique_ptr<Timer<T, TimeType>>;

template<typename _Tp>
  struct isDuration
  : std::false_type
  { };

template<typename _Rep, typename _Period>
  struct isDuration<std::chrono::duration<_Rep, _Period>>
  : std::true_type
  { };

template <typename T, typename TimeType = std::chrono::milliseconds>

#if __cplusplus > 201703L   // C++20
requires isDuration<TimeType>::value
#endif

class TimerManager
{
public:
    static_assert(isDuration<TimeType>::value, "TimeType must be duration.");

    using TimerType = Timer<T, TimeType>;

    explicit TimerManager() {}

    TimerType* addTimer(const TimeType& timeout, const T& data)
    {
#if TIMER_THREAD_SAFE
        std::unique_lock<std::mutex> lock(m_mutex);
#endif
        auto timer = new TimerType(timeout, data);
        m_queue.emplace(timer);
        return timer;
    }

    void checkout(std::vector<T>& list)
    {
#if TIMER_THREAD_SAFE
        std::unique_lock<std::mutex> lock(m_mutex);
#endif
        const auto now = std::chrono::system_clock::now();

        while(!m_queue.empty())
        {
            const auto &top = m_queue.top();

            if(top->isDisable())
            {
                m_queue.pop();
                continue;
            }
            else if(top->deadline() <= now)
            {
                list.emplace_back(top->userData());
                m_queue.pop();
            }
            else
                break;
        }
    }

    bool isEmpty() const { return m_queue.empty(); }

    T takeFirst()
    {
        while(true)
        {
            if(m_queue.empty())
                return {};
            else if(m_queue.top()->isDisable())
                m_queue.pop();
            else
                break;
        }

        T ret = m_queue.top()->userData();
        m_queue.pop();
        return ret;
    }

private:
#if TIMER_THREAD_SAFE
    std::mutex m_mutex;
#endif

    struct TimerCompare
    {
        inline bool operator() (const TimerItem<T, TimeType>& left,
                                const TimerItem<T, TimeType>& right)
        { return left->deadline() > right->deadline(); }
    };

    std::priority_queue<TimerItem<T, TimeType>,
                        std::deque<TimerItem<T, TimeType>>,
                        TimerCompare>
        m_queue;
};

#endif // TIMERMANAGER_H
