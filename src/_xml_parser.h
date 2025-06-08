#ifndef _XML_PARSER_H
#define _XML_PARSER_H 1

#include <assert.h>
#include <stdio.h>

#include "xml_parser.h"

#define XMLSetErrorMessage(PARSER, STRING) \
    do { \
        if ((PARSER)->isErrorAllocated == XML_TRUE) \
            (PARSER)->allocator->free((void *) (uintptr_t) (PARSER)->errorMessage, (PARSER)->allocator->ctx); \
        (PARSER)->errorMessage = (STRING); \
        (PARSER)->isErrorAllocated = XML_FALSE; \
    } while (0)

#define XMLAllocateErrorMessageChecked(PARSER, FMT, ...) \
    do { \
        char *str; \
        int size = snprintf(NULL, 0, (FMT), __VA_ARGS__); \
        if ( size < 0 || ( str = (PARSER)->allocator->malloc(size + 1, (PARSER)->allocator->ctx) ) == NULL ) { \
            XMLSetErrorMessage((PARSER), "out of memory\n"); \
            return XML_ERR_ALLOC; \
        } \
        if ( snprintf(str, size + 1, (FMT), __VA_ARGS__) != size ) { \
            (PARSER)->allocator->free(str, (PARSER)->allocator->ctx); \
            return XML_ERR_ALLOC; \
        } \
        (PARSER)->isErrorAllocated = XML_TRUE; \
        (PARSER)->errorMessage = (const char *) str; \
    } while (0)

#define XMLCheckError(F) \
    do { \
        XMLResult r = (F); \
        if ( r < 0 ) return r; \
    } while (0)

#define XMLCatchError(F, PARSER) \
    do { \
        XMLResult r = (F); \
        switch (r) { \
        case XML_SUCCESS: break; \
        case XML_ERR_ALLOC: \
            XMLSetErrorMessage((PARSER), "out of memory\n"); \
            return XML_ERR_ALLOC; \
        case XML_ERR_ENCODING: \
        case XML_ERR_OVERLONG: \
        case XML_ERR_SURROGATE: \
            XMLSetErrorMessage((PARSER), "invalid encoding\n"); \
            return XML_ERR_ENCODING; \
        case XML_ERR_INVALID_ARG: \
            return r; \
        } \
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
        } else if ( ( (ADVANCE) = (PARSER)->codec.decodeCodePoint((PARSER)->bytes.buf, (PARSER->bytes.pos), (PARSER)->bytes.len, &(CP)) ) < 0 ) { \
            XMLSetErrorMessage((PARSER), "invalid encoding\n"); \
            return XML_ERR_ENCODING; \
        } \
    } while (0)

#define XMLExpectCP(PARSER, CP, RESULT) \
    do { \
        XMLPeekCP(PARSER, (PARSER)->cp, RESULT); \
        if ( (PARSER)->cp != (CP) ) \
            (RESULT) = 0; \
        else { \
            (PARSER)->bytes.pos += (RESULT); \
            (PARSER)->bytes.atEnd = (PARSER)->bytes.pos >= (PARSER)->bytes.len ? XML_TRUE : XML_FALSE; \
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
XMLParser 
{
    XMLBoolean ignoreComments;
    XMLAllocator *allocator;
    XMLByteArray bytes;
    UnicodeCodec codec; 
    unsigned cp;
    XMLNode *root, *node;
    XMLBoolean isErrorAllocated;
    const char *errorMessage;
} XMLParser;

XMLBoolean
XMLIsNameStartChar(unsigned cp);

XMLBoolean
XMLIsNameChar(unsigned cp);

#define XMLConsumeWhiteSpaceChecked(PARSER, RESULT) \
    do { \
        if ( ( (RESULT) = XMLConsumeWhiteSpace((PARSER)) ) < 0 ) \
            return (RESULT); \
    } while (0)

XMLResult 
XMLConsumeWhiteSpace(XMLParser *parser);

XMLResult
XMLParseStartTag(XMLParser *parser);

XMLResult
XMLParseEndTag(XMLParser *parser, XMLString *endTag);

#endif
