/**
 * @author Ho 229
 * @date 2021/7/27
 */

#ifndef UNTIL_H
#define UNTIL_H

#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

#pragma warning(disable:4996)

namespace Until
{
    /**
     * @return Current time in GMT format
     */
    inline const std::string currentDateString()
    {
        const auto now = std::chrono::system_clock::now();
        const auto itt = std::chrono::system_clock::to_time_t(now);

        std::ostringstream ss;
        ss << std::put_time(gmtime(&itt), "%a, %d %b %Y %H:%M:%S GMT");
        return ss.str();
    }

    template <typename T>
    inline void toHex(std::string& buf, T num)
    {
        std::ostringstream stream;
        stream << std::hex << num;
        buf += stream.str();
    }
}

#endif // UNTIL_H
