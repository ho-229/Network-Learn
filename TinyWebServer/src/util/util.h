/**
 * @author Ho 229
 * @date 2021/7/27
 */

#ifndef UTIL_H
#define UTIL_H

#include <string_view>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

#ifndef _WIN32
#define gmtime_s(x, y) gmtime_r(y, x)
#endif

namespace Util
{
    /**
     * @brief overloaded
     */
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    /**
     * @return Current time in GMT format
     */
    inline const std::string currentDateString()
    {
        const auto now = std::chrono::system_clock::now();
        const auto itt = std::chrono::system_clock::to_time_t(now);

        std::tm gmt;
        std::ostringstream ss;
        gmtime_s(&gmt, &itt);
        ss << std::put_time(&gmt, "%a, %d %b %Y %H:%M:%S GMT");
        return ss.str();
    }

    inline constexpr int strcasecmp(const std::string_view &left,
                                    const std::string_view &right)
    {
        if(const auto diff = left.size() - right.size(); diff)
            return int(diff);

#ifdef _WIN32
        return _strnicmp(left.data(), right.data(), left.size());
#else
        return ::strncasecmp(left.data(), right.data(), left.size());
#endif
    }

    template <size_t maxCount = 0, typename Compare>
    bool referTil(std::string::size_type &offset,
                  std::string_view &dst, const std::string &src,
                  const Compare &limit)
    {
        if(src.empty())
            return false;

        const size_t srcSize = src.size();
        const auto start = offset;
        size_t count = 0;

        for(; offset < srcSize; ++offset, ++count)
        {
            if constexpr(maxCount)
            {
                if(maxCount <= count)
                    break;
            }

            if(limit(src[offset]))
            {
                dst = {src.data() + start, count};
                return true;
            }
        }

        dst = {src.data() + start, count};
        return false;
    }

    template <typename T>
    inline void toHex(std::string& buf, T num)
    {
        std::ostringstream stream;
        stream << std::hex << num;
        buf += stream.str();
    }

    template <typename Func>
    class ScopeFunction
    {
        const Func m_func;

    public:
        explicit ScopeFunction(const Func &func) : m_func(func) {}
        ~ScopeFunction() { m_func(); }
    };
}

#endif // UTIL_H
