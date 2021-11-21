#ifndef HEADERMAP_H
#define HEADERMAP_H

#ifndef _WIN32
extern "C"
{
# include <strings.h>
}
#endif

#include <map>
#include <string>
#include <unordered_map>

#define HASH_MAP 1

#if HASH_MAP
struct NocaseCompare
{
    inline bool operator()(const std::string &left,
                           const std::string &right) const
    {
# ifdef _WIN32
        return !_stricmp(left.c_str(), right.c_str());
# else
        return !strcasecmp(left.c_str(), right.c_str());
# endif
    }
};

struct NocaseHash
{
    size_t operator()(const std::string &str) const
    {
        if(str.empty())
            return 0;

        char first = str.front();

        if(first >= 'A' && first <= 'Z')    // to lower
            first += 'a' - 'A';
        else if(first < 'a' || first > 'z') // other charactor
            return 0;

        return size_t(first - 'a');
    }
};

typedef std::unordered_map<std::string,     // Name
                           std::string,     // Value
                           NocaseHash,      // Hash
                           NocaseCompare>   // Compare
    HeaderMap;

#else
struct NocaseCompare
{
    inline bool operator()(const std::string &left,
                           const std::string &right) const
    {
# ifdef _WIN32
        return _stricmp(left.c_str(), right.c_str()) < 0;
# else
        return strcasecmp(left.c_str(), right.c_str()) < 0;
# endif
    }
};

typedef std::map<std::string,   // Name
                 std::string,   // Value
                 NocaseCompare> // Compare
    HeaderMap;

#endif

#endif // HEADERMAP_H
