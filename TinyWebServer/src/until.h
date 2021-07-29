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

#ifdef _WIN32
# if _MSC_VER >= 1600
#  pragma execution_character_set("utf-8")
# endif
#endif

namespace Until
{
    /**
     * @return Current time in GMT format
     */
    inline const std::string currentDateString()
    {
        auto now = std::chrono::system_clock::now();
        auto itt = std::chrono::system_clock::to_time_t(now);

        std::ostringstream ss;
        ss << std::put_time(gmtime(&itt), "%a, %d %b %Y %H:%M:%S GMT");
        return ss.str();
    }

    inline const std::string errorHtml(int state, const std::string& message)
    {
        return "<h2>Tiny Web Server</h2><h1>" + std::to_string(state)
               + " " + message + "<br>∑(っ°Д°;)っ<h1>";
    }
}

#endif // UNTIL_H
