#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "xml_parse.h"
#include "xml_unicode.h"

#include "_xml_parser.h"

static void * 
XMLMalloc(size_t size, void *ctx)
{
    (void) ctx;
    return malloc(size);
}

static void *
XMLRealloc(void *p, size_t size, void *ctx)
{
    (void) ctx;
    return realloc(p, size);
}

static void
XMLFree(void *p, void *ctx)
{
    (void) ctx;
    free(p); 
}

XMLAllocator XMLDefaultAllocator = {
    .malloc  = XMLMalloc,
    .realloc = XMLRealloc,
    .free    = XMLFree,
    .ctx     = NULL
};

int 
InitXMLParser(XMLParserAttributes *attributes, XMLParser **out)
{    
    XMLAllocator *allocator = &XMLDefaultAllocator;
    _XMLParser *parser;
    if ( out == NULL )
        return XML_ERR_INVALID_ARG;
    if ( attributes != NULL && attributes->attribMask & XML_PARSER_ATTRIB_ALLOCATOR ) {
        if ( attributes->allocator == NULL )
            return XML_ERR_INVALID_ARG;
        allocator = attributes->allocator;
    }
    parser = allocator->malloc(sizeof(_XMLParser), allocator->ctx);
    if ( parser == NULL )
        return XML_ERR_ALLOC;
    memset(parser, 0, sizeof(_XMLParser));
    parser->base.ignoreComments = XML_TRUE;
    parser->base.allocator = allocator; 
    parser->codec = utf8Codec; 
    parser->isErrorAllocated = XML_FALSE;
    parser->errorMessage = NULL;
    if ( attributes != NULL ) {
        if ( attributes->attribMask & XML_PARSER_ATTRIB_IGNORE_COMMENTS ) {
            parser->base.ignoreComments = attributes->ignoreComments;
        }
    }
    *out = &parser->base;
    return XML_SUCCESS;
}

int 
ParseXML(XMLParser *base, const char *src, size_t len, XMLDocument *document)
{
    _XMLParser *parser;
    int result;
    char msg[8]; 
    if ( base == NULL )
        return XML_ERR_INVALID_PARSER;
    else if ( src == NULL || document == NULL )
        return XML_ERR_INVALID_ARG;
    parser = (_XMLParser *) base;
    parser->bytes = (XMLByteArray) {
        .atEnd = ( len == 0 ),
        .pos = 0,
        .len = len,
        .buf = (const unsigned char *) src,
    };
    document->version = XML_VERSION_1_0;
    // FIXME: properly parse encoding
    document->encoding = XML_ENCODING_UTF8;
    document->root = NULL;
    document->allocator = parser->base.allocator;
    XMLExpectCP(parser, XML_UNICODE_LESS_THAN, result);
    if ( result == 0 ) {
        XMLCharToString(msg, parser->codec, parser->cp);
        XMLAllocateErrorMessageChecked(parser, "expected opening tag '<': got %s\n", msg);
        return XML_ERR_PARSE;
    }
    if ( ( result = XMLParseStartTag(base) ) != XML_SUCCESS )
        return result;
    if ( parser->node != NULL ) {
        XMLAllocateErrorMessageChecked(parser, "unclosed tag <%s>\n", parser->node->name.buf);
        return XML_ERR_PARSE;
    }
    document->root = parser->root;
    return XML_SUCCESS;
    XMLExpectFail:
    XMLSetErrorMessage(parser, "expected opening tag '<'\n");
    return XML_ERR_PARSE;
    XMLAllocateFail:
    XMLSetErrorMessage(parser, "failed to allocate memory\n");
    return XML_ERR_ALLOC;
}

int
XMLNodeAppendChild(XMLNode *parent, XMLNode child, XMLAllocator *allocator)
{
    XMLNodeChildren *children;
    size_t cap;
    assert(parent != NULL);
    assert(parent->nodeType == XML_NODE_TYPE_MIXED); 
    assert(allocator != NULL);
    children = &parent->children;
    cap = children->cap;
    if ( cap <= children->nchildren ) {
        if ( cap == 0 )
            children->cap = 1;
        while ( children->cap <= children->nchildren )
            children->cap *= 2;
    }
    if ( cap != children->cap ) {
        XMLNode *nodes = allocator->realloc(children->nodes, sizeof(XMLNode) * children->cap, allocator->ctx);
        if ( nodes == NULL )
            return XML_ERR_ALLOC;
        children->nodes = nodes;
    }
    children->nodes[children->nchildren++] = child;
    return XML_SUCCESS;   
}

int
XMLNodeAppendAttrib(XMLNode *node, XMLNodeAttribute attrib, XMLAllocator *allocator)
{   
    size_t cap; 
    assert(node != NULL);
    assert(allocator != NULL); 
    cap = node->cattribs;
    if ( cap <= node->nattribs ) {
        if ( cap == 0 )
            node->cattribs = 1;
        while ( node->cattribs <= node->nattribs )
            node->cattribs *= 2;
    }
    if ( cap != node->cattribs ) {
        XMLNodeAttribute *attribs = allocator->realloc(node->attribs, sizeof(XMLNodeAttribute) * node->cattribs, allocator->ctx);
        if ( attribs == NULL )
            return XML_ERR_ALLOC;
        node->attribs = attribs;
    }
    node->attribs[node->nattribs++] = attrib;
    return XML_SUCCESS;   
}

const char *
GetXMLParserError(XMLParser *base)
{
    _XMLParser *parser;
    if ( base == NULL )
        return "XMLParser is NULL.";
    parser = (_XMLParser *) base;
    return parser->errorMessage; 
}

int 
DestroyXMLParser(XMLParser *base)
{
    _XMLParser *parser;
    XMLAllocator *allocator;
    if ( base == NULL )
        return XML_ERR_INVALID_ARG;
    parser = (_XMLParser *) base; 
    allocator = parser->base.allocator;
    if ( parser->isErrorAllocated )
        allocator->free((void *) (uintptr_t) parser->errorMessage, allocator->ctx);
    allocator->free(parser, allocator->ctx); 
    return XML_SUCCESS;
}
