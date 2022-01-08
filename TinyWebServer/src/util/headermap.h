/**
 * @author Ho 229
 * @date 2021/11/21
 */

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
template <typename T>
struct NocaseCompare
{
    inline bool operator()(const T &left,
                           const T &right) const
    {
# ifdef _WIN32
        return !_stricmp(left.data(), right.data());
# else
        return !strcasecmp(left.data(), right.data());
# endif
    }
};

template <typename T>
struct NocaseHash
{
    size_t operator()(const T &str) const
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

template <typename Key, typename Value>
using HeaderMap = std::unordered_map<Key, Value,
                                     NocaseHash<Key>,           // Hash
                                     NocaseCompare<Value>>;     // Compare;

#else
template <typename T>
struct NocaseCompare
{
    inline bool operator()(const T &left,
                           const T &right) const
    {
# ifdef _WIN32
        return _stricmp(left.data(), right.data()) < 0;
# else
        return strcasecmp(left.data(), right.data()) < 0;
# endif
    }
};

template <typename Key, typename Value>
using HeaderMap = std::map<Key, Value,
                           NocaseCompare<Value>>;
#endif

#endif // HEADERMAP_H
