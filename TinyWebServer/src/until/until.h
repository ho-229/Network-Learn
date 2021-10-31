﻿/**
 * @author Ho 229
 * @date 2021/7/27
 */

#ifndef UNTIL_H
#define UNTIL_H

#include <string_view>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

#ifndef _WIN32
#define gmtime_s(x, y) gmtime_r(y, x)
#endif

namespace Until
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

    template <typename Str>
    std::string_view getLine(std::string::size_type &offset,
                             const Str &text,
                             const std::string &limit)
    {
        if(offset >= text.size())       // Out of range
            return {};

        const auto pos = text.find(limit, offset);
        if(pos == std::string::npos)    // Not found
        {
            const auto begin = offset;
            offset = text.size();
            return {text.data() + begin};
        }

        std::string_view ret(text.data() + offset, pos - offset);
        offset += pos - offset + limit.size();

        return ret;
    }

    template <typename T>
    inline void toHex(std::string& buf, T num)
    {
        std::ostringstream stream;
        stream << std::hex << num;
        buf += stream.str();
    }

    template <typename Str>
    inline void toLower(Str& str)
    { std::transform(str.begin(), str.end(), str.begin(), tolower); }
}

#endif // UNTIL_H