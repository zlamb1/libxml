#include "_libxml.h"

static xmlParserAttributes defaultAttribs = {
    .allocator    = NULL,
    .srcConverter = &utf8Converter,
    .dstConverter = &utf8Converter,
};

xmlError
xmlInitParser(xmlParserAttributes *attribs, xmlParser **out)
{
    xmlBoolean isStdAllocator = LXML_FALSE;
    xmlParser *parser;
    xmlAllocator *allocator;
    if ( out == NULL )
        return LXML_ERR_ARG;
    if ( attribs == NULL )
        attribs = &defaultAttribs;
    allocator = attribs->allocator;
    if ( allocator == NULL ) {
        isStdAllocator = LXML_TRUE;
        XML_RETURN_ERROR(xmlInitStdAllocator(&allocator));
    } else allocator = attribs->allocator;
    parser = allocator->malloc(sizeof(xmlParser), allocator->ctx);
    if ( parser == NULL ) {
        if ( isStdAllocator == LXML_TRUE )
            XML_RETURN_ERROR(xmlDestroyStdAllocator(allocator));
        return LXML_ERR_ALLOC;
    }
    memset(parser, 0, sizeof(xmlParser));
    parser->isStdAllocator = isStdAllocator;
    parser->allocator = allocator;
    parser->srcConverter = attribs->srcConverter;
    if ( parser->srcConverter == NULL )
        parser->srcConverter = &utf8Converter;
    parser->dstConverter = attribs->dstConverter;
    if ( parser->dstConverter == NULL )
        parser->dstConverter = &utf8Converter;
    *out = parser;
    return LXML_ERR_NONE;
}

xmlError
xmlDestroyParser(xmlParser *parser)
{
    xmlBoolean isStdAllocator;
    xmlAllocator *allocator;
    if ( parser == NULL )
        return LXML_ERR_ARG;
    isStdAllocator = parser->isStdAllocator;
    allocator = parser->allocator;
    allocator->free(parser, allocator->ctx);
    if ( isStdAllocator == LXML_TRUE )
        XML_RETURN_ERROR(xmlDestroyStdAllocator(allocator));
    return LXML_ERR_NONE;
}
