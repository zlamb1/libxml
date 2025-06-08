#include "xml_parser.h"
#include "xml_string.h"
#include "xml_unicode.h"

XMLString
InitXMLString(UnicodeCodec codec, XMLAllocator *allocator)
{
    return (XMLString) {
        .len = 0,
        .bytes = 0,
        .cap = 0,
        .buf = NULL,
        .codec = codec,
        .allocator = allocator
    };
}

#include <stdio.h>

int
XMLStringAppend(XMLString *xmlString, unsigned codePoint)
{
    size_t cap, maxsize;
    unsigned encoded;
    int sz;
    if ( xmlString == NULL || xmlString->allocator == NULL )
        return XML_ERR_INVALID_ARG;
    if ( ( sz = xmlString->codec.encodeCodePoint(codePoint, &encoded) ) < 0 )
        return sz;
    cap = xmlString->cap;
    maxsize = xmlString->bytes + sz + 1;
    if ( cap < maxsize ) {
        if ( cap == 0 ) 
            xmlString->cap = 1;
        while ( xmlString->cap < maxsize )
            xmlString->cap *= 2;
    }
    if ( xmlString->cap != cap ) {
        char *buf = xmlString->allocator->realloc(xmlString->buf, xmlString->cap, xmlString->allocator->ctx);
        if ( buf == NULL )
            return XML_ERR_ALLOC;
        xmlString->buf = buf;
    }
    while ( sz-- ) {
        xmlString->buf[xmlString->bytes++] = encoded & 0xFF;
        encoded >>= 8;
    }
    xmlString->buf[xmlString->bytes] = '\0';
    xmlString->len++;
    return XML_SUCCESS;
}

int
DestroyXMLString(XMLString *xmlString)
{
    if ( xmlString == NULL || xmlString->allocator == NULL )
        return XML_ERR_INVALID_ARG;
    if ( xmlString->buf != NULL ) {
        xmlString->allocator->free(xmlString->buf, xmlString->allocator->ctx);
    }
    return XML_SUCCESS;
}
