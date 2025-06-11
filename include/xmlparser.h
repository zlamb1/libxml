#ifndef XMLPARSER_H
#define XMLPARSER_H 1

#include "xmlalloc.h"
#include "xmlencoding.h"

typedef enum
xmlWhitespacePolicy
{
    LXML_WHITESPACE_STRIP_NONE             = 0x0,
    LXML_WHITESPACE_STRIP_LEADING          = 0x1,
    LXML_WHITESPACE_STRIP_TRAILING         = 0x2,
    LXML_WHITESPACE_STRIP_LEADING_TRAILING = 0x3,
    LXML_WHITESPACE_STRIP_CONTENT          = 0x4,
    LXML_WHITESPACE_STRIP_ALL              = 0x7
} xmlWhitespacePolicy;

typedef enum
xmlStringInterningPolicy
{
    LXML_STRING_INTERN_NONE          = 0x0,
    LXML_STRING_INTERN_TAG_NAMES     = 0x1,
    LXML_STRING_INTERN_ATTRIB_NAMES  = 0x2,
    LXML_STRING_INTERN_ATTRIB_VALUES = 0x4
} xmlStringInterningPolicy;

typedef enum
xmlCommentPolicy
{
    LXML_COMMENT_PRESERVE_NONE     = 0x0,
    LXML_COMMENT_PRESERVE_LEADING  = 0x1,
    LXML_COMMENT_PRESERVE_DOCUMENT = 0x2,
    LXML_COMMENT_PRESERVE_TRAILING = 0x4,
    LXML_COMMENT_PRESERVE_ALL      = 0x7
} xmlCommentPolicy;

typedef struct 
xmlParserAttributes
{
    xmlAllocator *allocator;
    xmlEncodingConverter *srcConverter;
    xmlEncodingConverter *dstConverter;
    xmlWhitespacePolicy whiteSpacePolicy;
    xmlStringInterningPolicy stringInterningPolicy;
    xmlCommentPolicy commentPolicy;
} xmlParserAttributes;

typedef struct xmlParser xmlParser;

xmlError
xmlInitParser(xmlParserAttributes *attribs, xmlParser **parser);

xmlError
xmlDestroyParser(xmlParser *parser);

#endif
