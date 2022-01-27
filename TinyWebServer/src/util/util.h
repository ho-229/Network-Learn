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

    inline const std::string toGmtFormat(const std::time_t time)
    {
        std::tm gmt;
        std::ostringstream ss;
        gmtime_s(&gmt, &time);
        ss << std::put_time(&gmt, "%a, %d %b %Y %H:%M:%S GMT");
        return ss.str();
    }

    /**
     * @return Current time in GMT format
     */
    inline const std::string currentDateString()
    {
        const auto now = std::chrono::system_clock::now();
        const auto itt = std::chrono::system_clock::to_time_t(now);

        return toGmtFormat(itt);
    }

    /**
     * @ref https://stackoverflow.com/questions/61030383/how-to-convert-stdfilesystemfile-time-type-to-time-t
     */
    template <typename TP>
    std::time_t to_time_t(TP tp)
    {
        using namespace std::chrono;
        auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
                                                            + system_clock::now());
        return system_clock::to_time_t(sctp);
    }

    template <typename String>
    inline constexpr int strcasecmp(const String &left,
                                    const String &right)
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
