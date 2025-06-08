#ifndef XML_PARSER_H
#define XML_PARSER_H 1

#include <stdlib.h>

#include "xml_string.h"
#include "xml_type.h"

typedef enum
XMLParserAttributeMask
{
    XML_PARSER_ATTRIB_IGNORE_COMMENTS = 0x1,
    XML_PARSER_ATTRIB_ALLOCATOR       = 0x2
} XMLParserAttributeMask;

typedef struct
XMLParserAttributes
{
    XMLParserAttributeMask attribMask;
    XMLBoolean ignoreComments;
    XMLAllocator *allocator;
} XMLParserAttributes;

typedef struct XMLParser XMLParser;

typedef enum
XMLNodeType
{
    XML_NODE_TYPE_EMPTY,
    XML_NODE_TYPE_TEXT,
    XML_NODE_TYPE_COMMENT,
    XML_NODE_TYPE_MIXED
} XMLNodeType;

typedef struct
XMLNodeChildren
{
    size_t nchildren, cap;
    struct XMLNode *nodes;
} XMLNodeChildren;

typedef struct
XMLNodeAttribute
{
    XMLString name;
    XMLString value;
} XMLNodeAttribute;

typedef struct
XMLNode
{
    XMLString name;
    XMLBoolean isClosed;
    size_t nattribs, cattribs;
    XMLNodeAttribute *attribs;
    struct XMLNode *parent;
    XMLNodeType nodeType;
    union {
        XMLString text;
        XMLNodeChildren children;
    };
} XMLNode;

typedef enum
XMLDocumentVersion
{
    XML_VERSION_1_0
} XMLDocumentVersion;

typedef enum
XMLDocumentEncoding
{
    XML_ENCODING_UTF8,
    XML_ENCODING_UTF16,
    XML_ENCODING_UTF32
} XMLDocumentEncoding;

typedef struct
XMLDocument
{
    XMLDocumentVersion version;
    XMLDocumentEncoding encoding;
    XMLNode *root;
    XMLAllocator *allocator;
} XMLDocument;

XMLResult 
InitXMLParser(XMLParserAttributes *attributes, XMLParser **out);

XMLResult 
ParseXML(XMLParser *parser, const char *src, size_t len, XMLDocument *out); 

XMLResult
XMLNodeAppendChild(XMLNode *parent, XMLNode child, XMLAllocator *allocator);

XMLResult
XMLNodeAppendAttrib(XMLNode *node, XMLNodeAttribute attrib, XMLAllocator *allocator);

const char *
GetXMLParserError(XMLParser *parser);

XMLResult 
DestroyXMLParser(XMLParser *xmlParser);

XMLResult
DestroyXMLDocument(XMLDocument *document);

#endif
