#include <assert.h>

#include "xmlparser.h"

typedef struct XMLParser {
    XMLParserAllocator allocator;
} XMLParser;

static void * 
XMLMalloc(size_t size, void *ctx)
{
    (void) ctx;
    return malloc(size);
}

static void
XMLFree(void *p, void *ctx)
{
    (void) ctx;
    free(p); 
}

static XMLParserAllocator XMLDefaultAllocator = {
    .malloc = XMLMalloc,
    .free   = XMLFree,
    .ctx    = NULL
};

int 
InitXMLParser(XMLParserAttributes *attributes, XMLParser **out)
{
    XMLParserAllocator allocator = XMLDefaultAllocator;
    XMLParser *parser;
    assert(out != NULL);
    if ( attributes != NULL ) {
        if ( attributes->attributeMask & XML_PARSER_ATTRIBUTE_ALLOCATOR ) {
            allocator = attributes->allocator;
        }
    }
    parser = allocator.malloc(sizeof(XMLParserAllocator), allocator.ctx);
    parser->allocator = allocator; 
    if ( parser == NULL )
        return XML_ERR_ALLOC;
    *out = parser;
    return XML_SUCCESS;
}

int 
ParseXML(XMLParser *parser, const char *src, XMLDocument *out)
{
    assert(parser != NULL);
    assert(src != NULL); 
    assert(out != NULL);
    return XML_SUCCESS;
}

void DestroyXMLParser(XMLParser *xmlParser)
{
    XMLParserAllocator allocator;
    assert(xmlParser != NULL);
    allocator = xmlParser->allocator;
    allocator.free(xmlParser, allocator.ctx); 
}
