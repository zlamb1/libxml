#ifndef XML_PARSER_H
#define XML_PARSER_H 1

#include <stdlib.h>

#define XML_SUCCESS                0
#define XML_ERR_ALLOC             -1
#define XML_ERR_INVALID_DOCUMENT  -2

typedef struct
XMLParserAllocator
{
    void *(*malloc)(size_t size, void *ctx);
    void  (*free)(void *p, void *ctx); 
    void *ctx;
} XMLParserAllocator;

typedef enum
XMLParserAttributeMask
{
    XML_PARSER_ATTRIBUTE_ALLOCATOR = 0x1
} XMLParserAttributeMask;

typedef struct
XMLParserAttributes
{
    XMLParserAttributeMask attributeMask;
    XMLParserAllocator allocator;
} XMLParserAttributes;

typedef struct XMLParser XMLParser;

typedef enum
XMLNodeType
{
    XML_NODE_TYPE_EMPTY,
    XML_NODE_TYPE_TEXT,
    XML_NODE_TYPE_MIXED
} XMLNodeType;

typedef struct
XMLNodeChildren
{
    size_t nchildren;
    struct XMLNode *nodes;
} XMLNodeChildren;

typedef struct
XMLNodeAttribute
{
    char *name;
    char *value;
} XMLNodeAttribute;

typedef struct
XMLNode
{
    XMLNodeType nodeType;
    union {
        char *text;
        XMLNodeChildren children;
    };
} XMLNode;

typedef enum
XMLDocumentVersion
{
    XML_VERSION_1_0
} XMLDocumentVersion;

typedef struct
XMLDocument
{
    XMLDocumentVersion version;
    XMLNode *root;
} XMLDocument;

int 
InitXMLParser(XMLParserAttributes *attributes, XMLParser **out);

int 
ParseXML(XMLParser *parser, const char *src, XMLDocument *out); 

void DestroyXMLParser(XMLParser *xmlParser);

#endif
