#include "xmlencoding.h"

#define XML_VALIDATE_CONT_BYTE(BYTE) \
    do { \
        if ( ((BYTE) & 0xC0) != 0x80 ) { \
            decode.error = LXML_ERR_DECODE; \
            decode.scalar = 0; \
            return decode; \
        } \
    } while ( 0 )

#define XML_VALIDATE_OVERLONG(SCALAR, MIN) \
    do { \
        if ( (SCALAR) < (MIN) ) { \
            decode.error = LXML_ERR_OVERLONG; \
            return decode; \
        } \
    } while ( 0 )

static xmlScalarEncode
xmlEncodeUTF8(xmlUTF32 scalar, xmlUChar *dst, xmlSize len);

static xmlScalarDecode 
xmlDecodeUTF8(xmlUChar *src, xmlSize len);

xmlEncodingConverter utf8Converter = {
    .encode = xmlEncodeUTF8,
    .decode = xmlDecodeUTF8
};

xmlScalarEncode
xmlEncodeUTF8(xmlUTF32 scalar, xmlUChar *dst, xmlSize len)
{
    xmlScalarEncode encode = {0};
    if ( scalar <= 0x07F ) {
        encode.size = 1;
    } else if ( scalar <= 0x7FF ) {
        encode.size = 2;
    } else if ( scalar <= 0xFFFF ) {
        if ( scalar >= 0xD800 && scalar <= 0xDFFF ) {
            encode.error = LXML_ERR_SURROGATE;
            return encode;
        }
        encode.size = 3;
    } else if ( scalar <= 0x10FFFF ) {
        encode.size = 4;
    } else {
        encode.error = LXML_ERR_ARG;
        return encode;
    }
    if ( encode.size > len ) {
        encode.error = LXML_ERR_LEN;
        return encode;
    } else if ( dst == NULL ) {
        encode.error = LXML_ERR_ARG;
        return encode;
    }
    switch ( encode.size ) {
        case 1:
            *dst   = (xmlUChar) scalar;
            break;
        case 2:
            *dst++ = (xmlUChar) ((scalar >> 0x06) | 0xC0);
            *dst   = (xmlUChar) ((scalar  & 0x3F) | 0x80);
            break;
        case 3:
            *dst++ = (xmlUChar) (( scalar >> 0x0C) | 0xE0);
            *dst++ = (xmlUChar) (((scalar >> 0x06) & 0x3F) | 0x80);
            *dst   = (xmlUChar) (( scalar  & 0x3F) | 0x80);
            break;
        case 4:
            *dst++ = (xmlUChar) (( scalar >> 0x12) | 0xF0);
            *dst++ = (xmlUChar) (((scalar >> 0x0C) & 0x3F) | 0x80);
            *dst++ = (xmlUChar) (((scalar >> 0x06) & 0x3F) | 0x80);
            *dst   = (xmlUChar) (( scalar  & 0x3F) | 0x80);
            break;
    }
    encode.error = LXML_ERR_NONE;
    return encode;
}

xmlScalarDecode 
xmlDecodeUTF8(xmlUChar *src, xmlSize len)
{
    xmlScalarDecode decode = {0};
    xmlUChar ch;
    if ( src == NULL || len == 0 ) {
        decode.error = LXML_ERR_ARG;
        return decode;
    }
    ch = *src;
    if ( ( ch & 0x80 ) == 0 ) {
        decode.size = 1;
    } else if ( ( ch & 0xE0 ) == 0xC0 ) {
        decode.size = 2;
    } else if ( ( ch & 0xF0 ) == 0xE0 ) {
        decode.size = 3;
    } else if ( ( ch & 0xF8 ) == 0xF0 ) {
        decode.size = 4;
    } else {
        decode.error = LXML_ERR_ARG;
        return decode;
    }
    if ( decode.size > len ) {
        decode.error = LXML_ERR_LEN;
        return decode;
    }
    switch ( decode.size ) {
        case 1:
            decode.scalar |= ch;
            break;
        case 2:
            decode.scalar |= (*src++ & 0x1F) << 6;
            XML_VALIDATE_CONT_BYTE(*src);
            decode.scalar |= (*src & 0x3F);
            XML_VALIDATE_OVERLONG(decode.scalar, 0x80);
            break;
        case 3:
            decode.scalar |= (*src++ & 0x0F) << 12;
            XML_VALIDATE_CONT_BYTE(*src);
            decode.scalar |= (*src++ & 0x3F) << 6;
            XML_VALIDATE_CONT_BYTE(*src);
            decode.scalar |= (*src   & 0x3F);
            XML_VALIDATE_OVERLONG(decode.scalar, 0x800);
            if ( decode.scalar >= 0xD800 && decode.scalar <= 0xDFFF ) {
                decode.error = LXML_ERR_SURROGATE;
                return decode;
            }
            break;
        case 4:
            decode.scalar |= (*src++ & 0x07) << 18;
            XML_VALIDATE_CONT_BYTE(*src);
            decode.scalar |= (*src++ & 0x3F) << 12;
            XML_VALIDATE_CONT_BYTE(*src);
            decode.scalar |= (*src++ & 0x3F) << 6;
            XML_VALIDATE_CONT_BYTE(*src);
            decode.scalar |= (*src   & 0x3F);
            XML_VALIDATE_OVERLONG(decode.scalar, 0x10000);
            if ( decode.scalar > 0x10FFFF ) {
                decode.error = LXML_ERR_DECODE;
                return decode;
            }
            break;
    }
    decode.error = LXML_ERR_NONE;
    return decode;
}
