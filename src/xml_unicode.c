#include <stdio.h>

#include "xml_unicode.h"
#include "xml_parser.h"

int
_XMLEncodeCodePoint_UTF8(unsigned cp, unsigned *encoded);

int 
_XMLDecodeCodePoint_UTF8(const unsigned char *buf, size_t pos, size_t len, unsigned *cp);

UnicodeCodec utf8Codec = {
    .encodeCodePoint = _XMLEncodeCodePoint_UTF8,
    .decodeCodePoint = _XMLDecodeCodePoint_UTF8
};

int
_XMLEncodeCodePoint_UTF8(unsigned cp, unsigned *encoded)
{
    int len;
    unsigned _encoded = 0;
    if ( cp >= 0x10000 && cp <= 0x10FFFF ) {
        len = 4;
        _encoded |= ((cp >> 18 & 0x07) | 0xF0);
        _encoded |= ((cp >> 12 & 0x3F) | 0x80) << 8;
        _encoded |= ((cp >> 6 & 0x3F) | 0x80) << 16;
        _encoded |= ((cp & 0x3F) | 0x80) << 24;
    } else if ( cp >= 0x800 && cp <= 0xFFFF ) {
        len = 3;
        _encoded |= ((cp >> 12 & 0x0F) | 0xE0);
        _encoded |= ((cp >> 6 & 0x3F) | 0x80) << 8;
        _encoded |= ((cp & 0x3F) | 0x80) << 16;
    } else if ( cp >= 0x80 && cp <= 0x7FF ) {
        len = 2;
        _encoded |= ((cp >> 6 & 0x1F) | 0xC0);
        _encoded |= ((cp & 0x3F) | 0x80) << 8;
    } else if ( cp < 0x80 ) {
        len = 1;
        _encoded |= cp;
    } else {
        return XML_ERR_ENCODING;
    }
    if ( encoded != NULL )
        *encoded = _encoded;
    return len;
}

int 
_XMLDecodeCodePoint_UTF8(const unsigned char *buf, size_t pos, size_t len, unsigned *cp)
{
    int size = 0;
    unsigned _cp = 0;
    if ( pos >= len )
        return XML_ERR_BUF_SIZE;
    if ( ( buf[pos] & 0xF8 ) == 0xF0 ) {
        if ( pos + 4 > len )
            return XML_ERR_BUF_SIZE;
        _cp |= (buf[pos++] & 0x7) << 18;
        if ( ( buf[pos] & 0xC0 ) != 0x80 )
            return XML_ERR_ENCODING;
        _cp |= (buf[pos++] & 0x3F) << 12;
        if ( ( buf[pos] & 0xC0 ) != 0x80 )
            return XML_ERR_ENCODING;
        _cp |= (buf[pos++] & 0x3F) << 6;
        if ( ( buf[pos] & 0xC0 ) != 0x80 )
            return XML_ERR_ENCODING;
        _cp |= (buf[pos] & 0x3F);
        if ( _cp < 0x010000 ) 
            return XML_ERR_OVERLONG;
        if ( _cp > 0x10FFFF ) 
            return XML_ERR_ENCODING;
        size = 4;
    } else if ( ( buf[pos] & 0xF0 ) == 0xE0 ) {
        if ( pos + 3 > len )
            return XML_ERR_BUF_SIZE;
        _cp |= (buf[pos++] & 0x0F) << 12;
        if ( ( buf[pos] & 0xC0 ) != 0x80 )
            return XML_ERR_ENCODING;
        _cp |= (buf[pos++] & 0x3F) << 6;
        if ( ( buf[pos] & 0xC0 ) != 0x80 )
            return XML_ERR_ENCODING;
        _cp |= (buf[pos] & 0x3F);
        if ( _cp < 0x0800 )                                        
            return XML_ERR_OVERLONG;
        if ( _cp > 0xFFFF || ( _cp > 0xD7FF && _cp < 0xE000 )  ) 
            return XML_ERR_ENCODING;
        size = 3;
    } else if ( ( buf[pos] & 0xE0 ) == 0xC0 ) {
        if ( pos + 2 > len )
            return XML_ERR_BUF_SIZE;
        _cp |= (buf[pos++] & 0x1F) << 6;
        if ( ( buf[pos] & 0xC0 ) != 0x80 )
            return XML_ERR_ENCODING;
        _cp |= (buf[pos] & 0x3F);
        if ( _cp < 0x080 ) return XML_ERR_OVERLONG;
        if ( _cp > 0x7FF ) return XML_ERR_ENCODING;
        size = 2;
    } else if ( ( buf[pos] & 0x80 ) == 0 ) {
        _cp |= buf[pos];
        size = 1;
    } else return XML_ERR_ENCODING;
    if (cp != NULL)
        *cp = _cp;
    return size;
}
