/**
 * @author Ho 229
 * @date 2021/8/20
 */

#ifndef TIMERMANAGER_H
#define TIMERMANAGER_H

#include <list>
#include <mutex>
#include <vector>
#include <atomic>
#include <chrono>
#include <memory>

#define TIMER_THREAD_SAFE 0

template <typename T, typename TimeType>
class Timer
{
public:
    explicit Timer(const TimeType& timeout, const T& data) :
        m_userData(std::move(data)),
        m_timeout(timeout),
        m_deadline(std::chrono::system_clock::now() + timeout)
    {}

    inline void reset()
    { m_deadline = std::chrono::system_clock::now() + m_timeout; }

    inline const auto& deadline() const { return m_deadline; }

    inline const T& userData() const { return m_userData; }

    inline bool operator<(const Timer<T, TimeType> &right) const
    { return this->m_deadline < right.m_deadline; }

private:
    const T m_userData;
    const TimeType& m_timeout;
    std::chrono::system_clock::time_point m_deadline;
};

template<typename Tp>
  struct isDuration
  : std::false_type
  { };

template<typename Rep, typename Period>
  struct isDuration<std::chrono::duration<Rep, Period>>
  : std::true_type
  { };

template <typename T, typename TimeType = std::chrono::milliseconds>

#if __cplusplus > 201703L   // C++20
requires isDuration<TimeType>::value && std::is_copy_constructible_v<T>
#endif

class TimerManager
{
public:
    static_assert(isDuration<TimeType>::value, "TimeType must be std::duration.");
    static_assert(std::is_copy_constructible_v<T>, "T must be copyable.");

    using TimerItem = Timer<T, TimeType>;
    using iterator = typename std::list<TimerItem>::iterator;

    explicit TimerManager() = default;

    iterator start(const TimeType& timeout, const T& data)
    {
#if TIMER_THREAD_SAFE
        std::unique_lock<std::mutex> lock(m_mutex);
#endif
        TimerItem timer(timeout, data);

        for(auto it = m_list.end(); it != m_list.begin(); --it)
        {
            if(*std::prev(it) < timer)
                return m_list.emplace(it, timer);
        }

        return m_list.emplace(m_list.begin(), timer);
    }

    void restart(iterator timerIt)
    {
#if TIMER_THREAD_SAFE
        std::unique_lock<std::mutex> lock(m_mutex);
#endif
        timerIt->reset();

        for(auto it = m_list.end(); it != m_list.begin(); --it)
        {
            if(*std::prev(it) < *timerIt)
                return m_list.splice(it, m_list, timerIt);
        }
    }

    void destory(iterator it)
    {
        if(!m_list.empty())
            m_list.erase(it);
    }

    void checkout(std::vector<T>& list)
    {
#if TIMER_THREAD_SAFE
        std::unique_lock<std::mutex> lock(m_mutex);
#endif
        const auto now = std::chrono::system_clock::now();

        while(!m_list.empty() && m_list.front().deadline() <= now)
        {
            list.emplace_back(m_list.front().userData());
            m_list.pop_front();
        }
    }

    bool isEmpty() const { return m_list.empty(); }

    size_t size() const { return m_list.size(); }

    T takeFirst()
    {
#if TIMER_THREAD_SAFE
        std::unique_lock<std::mutex> lock(m_mutex);
#endif
        if(m_list.empty())
            return {};

        T ret = m_list.front().userData();
        m_list.pop_front();

        return ret;
    }

private:
#if TIMER_THREAD_SAFE
    std::mutex m_mutex;
#endif

    std::list<TimerItem> m_list;
};

#endif // TIMERMANAGER_H
