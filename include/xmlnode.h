#ifndef XMLNODE_H
#define XMLNODE_H 1

#include "xmlstring.h"
#include "xmltype.h"

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

xmlError xmlNodeAppendChild (xmlNode *parent, xmlNode **out,
                             xmlAllocator *allocator);

xmlError xmlNodeGetTreeMemorySize (xmlNode *root, xmlSize *out);

xmlError xmlDestroyNode (xmlNode *root, xmlAllocator *allocator,
                         xmlBoolean destroyRoot);

#endif
