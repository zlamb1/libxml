#ifndef LIBXML_H
#define LIBXML_H 1

#include "xmlparser.h"
#include "xmlstring.h"

typedef struct xmlEntity
{
  xmlString name;
  xmlString value;
} xmlEntity;

typedef enum xmlNodeType
{
  LXML_NODE_TYPE_EMPTY,
  LXML_NODE_TYPE_MIXED,
  LXML_NODE_TYPE_TEXT,
  LXML_NODE_TYPE_COMMENT,
} xmlNodeType;

typedef struct xmlNodeAttribute
{
  xmlString name;
  xmlString value;
} xmlNodeAttribute;

typedef struct xmlNode
{
  xmlNodeType type;
  xmlString name;
  struct xmlNode *parent;
  xmlSize nattribs, cattribs;
  xmlNodeAttribute *attribs;
  union
  {
    struct
    {
      xmlSize nchildren, cchildren;
      struct xmlNode *children;
    };
    xmlString text;
    xmlString comment;
  };
} xmlNode;

typedef enum xmlEncoding
{
  LXML_ENCODING_UTF8,
  LXML_ENCODING_UTF16,
  LXML_ENCODING_UTF32
} xmlEncoding;

typedef enum xmlVersion
{
  LXML_VERSION_1_0,
  LXML_VERSION_1_1
} xmlVersion;

typedef struct xmlDocument xmlDocument;

xmlError xmlParseDocument (xmlChar *src, xmlSize size, xmlParser *parser,
                           xmlDocument **document);

xmlError xmlGetDocumentRoot (xmlDocument *document, xmlNode **out);

xmlError xmlGetDocumentStringIterator (xmlDocument *document,
                                       xmlString *string,
                                       xmlStringIterator *iterator);

xmlError xmlDestroyDocument (xmlDocument *document);

#endif
