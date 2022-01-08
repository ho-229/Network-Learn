/**
 * @author Ho 229
 * @date 2021/7/27
 */

#ifndef UTIL_H
#define UTIL_H

#include <string_view>
#include <algorithm>
#include <sstream>
#include <wchar.h>
#include <iomanip>
#include <chrono>
#include <ctime>

#ifndef _WIN32
#define gmtime_s(x, y) gmtime_r(y, x)
#endif

namespace Util
{
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

    inline std::wstring fromLocal8Bit(const std::string &src)
    {
        std::wstring ret;
        ret.resize(ret.size() + 1);
        mbstowcs(ret.data(), src.c_str(), src.size() + 1);

        return ret;
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
}

#endif // UTIL_H
