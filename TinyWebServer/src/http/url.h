/**
 * @ref https://github.com/ithewei/libhv/blob/6cf0ce0eb09caf779d5524c154d2166d9aab7299/cpputil/hurl.cpp
 */

#ifndef URL_H
#define URL_H

#include <string>

#define IS_ALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define IS_NUM(c)   ((c) >= '0' && (c) <= '9')
#define IS_HEX(c) (IS_NUM(c) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#define IS_ALPHANUM(c) (IS_ALPHA(c) || IS_NUM(c))

static inline bool isUnambiguous(const uint8_t c)
{
    return IS_ALPHANUM(c) ||
            c == '-' ||
            c == '_' ||
            c == '.' ||
            c == '~';
}

static inline unsigned char hex2i(const uint8_t hex)
{
    return hex <= '9' ? hex - '0' :
                        hex <= 'F' ? hex - 'A' + 10 : hex - 'a' + 10;
}

template <typename String>
std::string uriEscape(const String &src)
{
    std::string ostr;

    static const char tab[] = "0123456789ABCDEF";
    const auto it = src.cbegin();

    char szHex[4] = "%00";

    while(it != src.cend())
    {
        if(isUnambiguous(*it))
            ostr += char(*it);
        else
        {
            szHex[1] = tab[*it >> 4];
            szHex[2] = tab[*it & 0xF];
            ostr += szHex;
        }
        ++it;
    }

    return ostr;
}

template <typename String>
std::string uriUnescape(const String &src)
{
    std::string ostr;
    auto it = src.cbegin();

    while(it != src.cend())
    {
        if(*it == '%' && IS_HEX(it[1]) && IS_HEX(it[2]))
        {
            ostr += char((hex2i(it[1]) << 4) | hex2i(it[2]));
            it += 3;
        }
        else
        {
            ostr += char(*it);
            ++it;
        }
    }

    return ostr;
}

#endif // URL_H
