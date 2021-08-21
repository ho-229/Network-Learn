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
        ss << std::put_time(gmtime_s(&gmt, &itt), "%a, %d %b %Y %H:%M:%S GMT");
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
