#ifndef LIBXML_H
#define LIBXML_H 1

#include "xmlentity.h"
#include "xmlnode.h"
#include "xmlparser.h"

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
