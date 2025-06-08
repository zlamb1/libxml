#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "xml_unicode.h"

#include "_xml_parser.h"

#define XML_DEBUG_ALLOC

typedef struct 
XMLAllocation
{
    size_t size;
    void *ptr;
    struct XMLAllocation *next;
} XMLAllocation;

static size_t bytesAllocated = 0, bytesFreed = 0;
static XMLAllocation *allocations = NULL;

static void * 
XMLMalloc(size_t size, void *ctx)
{
#ifdef XML_DEBUG_ALLOC
    XMLAllocation *alloc;
    alloc = malloc(sizeof(XMLAllocation));
    alloc->size = size;
    alloc->ptr = malloc(size);
    alloc->next = allocations;
    allocations = alloc;
    bytesAllocated += size;
    return alloc->ptr;
#endif
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunreachable-code"
    (void) bytesAllocated;
    (void) bytesFreed;
    (void) allocations;
    (void) ctx;
    return malloc(size);
#pragma GCC diagnostic pop
}

static void *
XMLRealloc(void *p, size_t size, void *ctx)
{
#ifdef XML_DEBUG_ALLOC
    XMLAllocation *alloc = allocations;
    if ( p == NULL )
        return XMLMalloc(size, ctx);
    while ( alloc != NULL ) {
        if ( alloc->ptr == p ) {
            bytesAllocated += size;
            bytesFreed += alloc->size;
            alloc->size = size;
            alloc->ptr = realloc(p, size);
            return alloc->ptr;
        }
        alloc = alloc->next;
    }
    assert(0);
#endif
    (void) ctx;
    return realloc(p, size);
}

static void
XMLFree(void *p, void *ctx)
{
#ifdef XML_DEBUG_ALLOC
    XMLAllocation *prev = NULL, *alloc = allocations;
    while ( alloc != NULL ) {
        if ( alloc->ptr == p ) {
            if ( prev == NULL )
                allocations = alloc->next; 
            else prev->next = alloc->next;
            bytesFreed += alloc->size;
            free(p);
            free(alloc);
            return;
        }
        prev = alloc;
        alloc = alloc->next;
    }
    assert(0);
#endif
    (void) ctx;
    free(p); 
}

#ifdef XML_DEBUG_ALLOC
void _XMLPrintAllocTracking(void);

void 
_XMLPrintAllocTracking(void)
{
    printf("bytes allocated: %llu\n", bytesAllocated);
    printf("bytes freed: %llu\n", bytesFreed);
}
#endif

XMLAllocator XMLDefaultAllocator = {
    .malloc  = XMLMalloc,
    .realloc = XMLRealloc,
    .free    = XMLFree,
    .ctx     = NULL
};

XMLResult 
InitXMLParser(XMLParserAttributes *attributes, XMLParser **out)
{    
    XMLAllocator *allocator = &XMLDefaultAllocator;
    XMLParser *parser;
    if ( out == NULL )
        return XML_ERR_INVALID_ARG;
    if ( attributes != NULL && attributes->attribMask & XML_PARSER_ATTRIB_ALLOCATOR ) {
        if ( attributes->allocator == NULL )
            return XML_ERR_INVALID_ARG;
        allocator = attributes->allocator;
    }
    parser = allocator->malloc(sizeof(XMLParser), allocator->ctx);
    if ( parser == NULL )
        return XML_ERR_ALLOC;
    memset(parser, 0, sizeof(XMLParser));
    parser->ignoreComments = XML_TRUE;
    parser->allocator = allocator; 
    parser->codec = utf8Codec; 
    parser->isErrorAllocated = XML_FALSE;
    parser->errorMessage = NULL;
    if ( attributes != NULL ) {
        if ( attributes->attribMask & XML_PARSER_ATTRIB_IGNORE_COMMENTS ) {
            parser->ignoreComments = attributes->ignoreComments;
        }
    }
    *out = parser;
    return XML_SUCCESS;
}

XMLResult 
ParseXML(XMLParser *parser, const char *src, size_t len, XMLDocument *document)
{
    XMLResult result;
    char msg[8]; 
    if ( parser == NULL )
        return XML_ERR_INVALID_PARSER;
    else if ( src == NULL || document == NULL )
        return XML_ERR_INVALID_ARG;
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
    document->allocator = parser->allocator;
    while ( 1 ) {
        XMLExpectCP(parser, XML_UNICODE_LESS_THAN, result);
        if ( result == 0 ) {
            XMLCharToString(msg, parser->codec, parser->cp);
            XMLAllocateErrorMessageChecked(parser, "expected opening tag '<': got %s\n", msg);
            return XML_ERR_PARSE;
        }
        XMLPeekCP(parser, parser->cp, result);
        switch ( parser->cp ) {
            case XML_UNICODE_EXCLAMATION:
                // TODO: parse/discard comments
                break;
            case XML_UNICODE_SLASH: {
                XMLString startTag;
                XMLString endTag = InitXMLString(parser->codec, parser->allocator);
                XMLAdvanceCP(parser, result);
                if ( ( result = XMLParseEndTag(parser, &endTag) ) != XML_SUCCESS )
                    return result;
                if ( parser->node == NULL ) {
                    XMLAllocateErrorMessageChecked(parser, "no matching starting tag for </%s>\n", endTag.buf);
                    DestroyXMLString(&endTag);
                    return XML_ERR_PARSE;
                }
                startTag = parser->node->name;
                if ( startTag.bytes != endTag.bytes || memcmp(startTag.buf, endTag.buf, startTag.bytes) != 0 ) {
                    XMLAllocateErrorMessageChecked(parser, "expected end tag for <%s>: got </%s>\n", startTag.buf, endTag.buf);
                    DestroyXMLString(&endTag);
                    return XML_ERR_PARSE;
                }
                parser->node->isClosed = XML_TRUE;
                parser->node = parser->node->parent;
                DestroyXMLString(&endTag);
                break;
            }
            default:
                if ( ( result = XMLParseStartTag(parser) ) != XML_SUCCESS )
                    return result;
        }
        XMLConsumeWhiteSpaceChecked(parser, result);
        if ( parser->bytes.atEnd )
            break;
    }
    if ( parser->node != NULL ) {
        XMLAllocateErrorMessageChecked(parser, "unclosed tag <%s>\n", parser->node->name.buf);
        return XML_ERR_PARSE;
    }
    document->root = parser->root;
    return XML_SUCCESS;
}

XMLResult
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

XMLResult
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
GetXMLParserError(XMLParser *parser)
{
    if ( parser == NULL )
        return "XMLParser is NULL.";
    return parser->errorMessage; 
}

XMLResult 
DestroyXMLParser(XMLParser *parser)
{
    XMLAllocator *allocator;
    if ( parser == NULL )
        return XML_ERR_INVALID_PARSER;
    allocator = parser->allocator;
    if ( parser->isErrorAllocated )
        allocator->free((void *) (uintptr_t) parser->errorMessage, allocator->ctx);
    allocator->free(parser, allocator->ctx); 
    return XML_SUCCESS;
}

static XMLResult
_DestroyXMLNode(XMLNode *node, XMLAllocator *allocator)
{
    XMLResult result;
    switch ( node->nodeType ) {
        case XML_NODE_TYPE_TEXT:
            DestroyXMLString(&node->text);
            break;
        case XML_NODE_TYPE_MIXED:
            for ( unsigned i = 0; i < node->children.nchildren; i++) {
                result = _DestroyXMLNode(&node->children.nodes[i], allocator);
                if ( result != XML_SUCCESS )
                    return result;
            }
            break;
        default: 
            break;
    }
    if ( node->attribs != NULL )
        allocator->free(node->attribs, allocator->ctx);
    allocator->free(node, allocator->ctx);
    return XML_SUCCESS;
}

XMLResult
DestroyXMLDocument(XMLDocument *document)
{
    if ( document == NULL )
        return XML_ERR_INVALID_DOCUMENT;
    if ( document->root != NULL )
        return _DestroyXMLNode(document->root, document->allocator);
    return XML_SUCCESS;
}
