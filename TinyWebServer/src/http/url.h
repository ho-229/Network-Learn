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

std::string uriEscape(const std::string &src)
{
    std::string ostr;

    static const char tab[] = "0123456789ABCDEF";
    const uint8_t* p = reinterpret_cast<const uint8_t*>(src.data());

    char szHex[4] = "%00";

    while(*p != '\0')
    {
        if(isUnambiguous(*p))
            ostr += char(*p);
        else
        {
            szHex[1] = tab[*p >> 4];
            szHex[2] = tab[*p & 0xF];
            ostr += szHex;
        }
        ++p;
    }

    return ostr;
}

std::string uriUnescape(const std::string &src)
{
    std::string ostr;
    const uint8_t* p = reinterpret_cast<const uint8_t*>(src.data());

    while(*p != '\0')
    {
        if(*p == '%' && IS_HEX(p[1]) && IS_HEX(p[2]))
        {
            ostr += char((hex2i(p[1]) << 4) | hex2i(p[2]));
            p += 3;
        }
        else
        {
            ostr += char(*p);
            ++p;
        }
    }

    return ostr;
}

#endif // URL_H
