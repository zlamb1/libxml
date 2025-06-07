#ifndef _XML_PARSER_H
#define _XML_PARSER_H 1

#include <assert.h>
#include <stdio.h>

#include "xml_parser.h"

#define XMLSetErrorMessage(PARSER, STRING) \
    do { \
        (PARSER)->errorMessage = (STRING); \
        (PARSER)->isErrorAllocated = XML_FALSE; \
    } while (0)

#define XMLAllocateErrorMessageChecked(PARSER, FMT, ...) \
    do { \
        char *str; \
        int size = snprintf(NULL, 0, (FMT), __VA_ARGS__); \
        if ( size < 0 || ( str = (PARSER)->base.allocator->malloc(size + 1, (PARSER)->base.allocator->ctx) ) == NULL ) \
            goto XMLAllocateFail; \
        if ( snprintf(str, size + 1, (FMT), __VA_ARGS__) != size ) { \
            (PARSER)->base.allocator->free(str, (PARSER)->base.allocator->ctx); \
            goto XMLAllocateFail; \
        } \
        (PARSER)->isErrorAllocated = XML_TRUE; \
        (PARSER)->errorMessage = (const char *) str; \
    } while (0)

#define XMLAdvanceCP(PARSER, ADVANCE) \
    do { \
        assert((PARSER) != NULL); \
        assert((PARSER)->bytes.atEnd == XML_FALSE); \
        (PARSER)->bytes.pos += (ADVANCE); \
        (PARSER)->bytes.atEnd = (PARSER)->bytes.pos >= (PARSER)->bytes.len ? XML_TRUE : XML_FALSE; \
    } while (0)

#define XMLPeekCP(PARSER, CP, ADVANCE) \
    do { \
        if ( (PARSER)->bytes.atEnd == XML_TRUE ) { \
            (CP) = XML_UNICODE_NUL; \
            (ADVANCE) = 0; \
        } \
        else if ( ( (ADVANCE) = (PARSER)->codec.decodeCodePoint((PARSER)->bytes.buf, (PARSER->bytes.pos), (PARSER)->bytes.len, &(CP)) ) < 0 ) \
            goto XMLPeekFail; \
    } while (0)

#define XMLExpectCP(PARSER, CP, RESULT) \
    do { \
        if ( (PARSER)->bytes.atEnd == XML_TRUE ) { \
            (PARSER)->cp = 0; \
            (RESULT) = 0; \
        } else { \
            if ( ( (RESULT) = (PARSER)->codec.decodeCodePoint((PARSER)->bytes.buf, (PARSER)->bytes.pos, (PARSER)->bytes.len, &(PARSER)->cp) ) < 0 ) \
                goto XMLExpectFail; \
            assert((RESULT) != 0); \
            if ( (PARSER)->cp != (CP) ) \
                (RESULT) = 0; \
            else { \
                (PARSER)->bytes.pos += (RESULT); \
                (PARSER)->bytes.atEnd = (PARSER)->bytes.pos >= (PARSER)->bytes.len ? XML_TRUE : XML_FALSE; \
                assert((PARSER)->bytes.pos <= (PARSER)->bytes.len); \
            } \
        } \
    } while (0)

typedef struct
XMLByteArray
{
    XMLBoolean atEnd;
    size_t len, pos;
    const unsigned char *buf;
} XMLByteArray;

typedef struct 
_XMLParser 
{
    XMLParser base;
    XMLByteArray bytes;
    UnicodeCodec codec; 
    unsigned cp;
    XMLNode *root, *node;
    XMLBoolean isErrorAllocated;
    const char *errorMessage;
} _XMLParser;

#endif
