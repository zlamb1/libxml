#ifndef XML_STRING_H
#define XML_STRING_H 1

#include <stdlib.h>
#include <string.h>

#include "xml_allocator.h"
#include "xml_unicode.h"

#define XMLStringAppendChecked(STRING, CHAR) \
    do { \
        if ( XMLStringAppend((STRING), (CHAR)) != XML_SUCCESS ) \
            goto XMLAllocateFail; \
    } while (0)

#define XMLCharToString(BUF, CODEC, CHAR) \
    do { \
        _Static_assert(sizeof((BUF)) >= 8, "Minimum buffer size is 8."); \
        switch ( (CHAR) ) { \
            case XML_UNICODE_NUL: \
                memcpy((BUF), "EOF", sizeof("EOF")); \
                break; \
            case XML_UNICODE_TAB: \
                memcpy((BUF), "TAB", sizeof("TAB")); \
                break; \
            case XML_UNICODE_CR: \
                memcpy((BUF), "CR", sizeof("CR")); \
                break; \
            case XML_UNICODE_NEWLINE: \
                memcpy((BUF), "newline", sizeof("newline")); \
                break; \
            default: { \
                int sz, i = 1; \
                unsigned encoded; \
                (BUF)[0] = '\''; \
                if ( ( sz = (CODEC).encodeCodePoint((CHAR), &encoded) ) <= 0 ) { \
                    sz = 3; \
                    encoded = 0xFFFD; \
                } \
                while ( sz-- ) {\
                    BUF[i++] = ( encoded & 0xFF ); \
                    encoded >>= 8; \
                } \
                BUF[i++] = '\''; BUF[i++] = '\0'; \
                break; \
            } \
        } \
    } while (0)

typedef struct
XMLString
{
    size_t len, bytes, cap;
    char *buf;
    UnicodeCodec codec; 
    XMLAllocator *allocator;
} XMLString;

XMLString
InitXMLString(UnicodeCodec codec, XMLAllocator *allocator); 

int
XMLStringAppend(XMLString *xmlString, unsigned codePoint);

int
DestroyXMLString(XMLString *xmlString); 

#endif
