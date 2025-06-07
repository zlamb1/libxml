#ifndef XML_UTF8_H
#define XML_UTF8_H 1

#define XML_UNICODE_NUL           0x00  // \0
#define XML_UNICODE_TAB           0x09  // \t
#define XML_UNICODE_NEWLINE       0x0A  // \n
#define XML_UNICODE_CR            0x0D  // \r
#define XML_UNICODE_SPACE         0x20
#define XML_UNICODE_QUOTE         0x22  // "
#define XML_UNICODE_AMPERSAND     0x26  // & 
#define XML_UNICODE_APOSTROPHE    0x27  // '
#define XML_UNICODE_HYPHEN        0x2D  // -
#define XML_UNICODE_PERIOD        0x2E  // .
#define XML_UNICODE_SLASH         0x2F  // /
#define XML_UNICODE_COLON         0x3A  // :
#define XML_UNICODE_LESS_THAN     0x3C  // <
#define XML_UNICODE_EQUALS        0x3D  // =
#define XML_UNICODE_GREATER_THAN  0x3E  // >
#define XML_UNICODE_UNDERSCORE    0x5F  // _
#define XML_UNICODE_MIDDLE_POINT  0xB7  // ·

typedef struct 
UnicodeCodec
{
    int (*encodeCodePoint)(unsigned cp, unsigned *encoded); 
    int (*decodeCodePoint)(const unsigned char *buf, size_t pos, size_t len, unsigned *cp);
} UnicodeCodec;

extern UnicodeCodec utf8Codec; 

#endif
