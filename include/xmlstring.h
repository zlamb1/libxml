#include "xmlunicode.h"
#ifndef XMLSTRING_H
#define XMLSTRING_H 1

#include "xmlalloc.h"
#include "xmlencoding.h"
#include "xmltype.h"

typedef struct xmlString
{
  xmlSize len;
  xmlSize bytes;
  xmlSize cap;
  xmlChar *buf;
} xmlString;

typedef struct xmlStringIterator
{
  xmlEncodingConverter *encoding;
  xmlSize pos;
  xmlString *string;
} xmlStringIterator;

xmlError xmlStringAppend (xmlString *string, xmlAllocator *allocator,
                          xmlEncodingConverter *encoding, xmlUTF32 character,
                          xmlBoolean setBytes);

xmlError xmlStringClear (xmlString *string);

inline static xmlError
xmlStringTerminate (xmlString *string, xmlAllocator *allocator,
                    xmlEncodingConverter *encoding)
{
  return xmlStringAppend (string, allocator, encoding, LXML_UC_NUL,
                          LXML_FALSE);
}

xmlError xmlDestroyString (xmlString *string, xmlAllocator *allocator);

xmlError xmlStringIteratorNext (xmlStringIterator *iterator, xmlUTF32 *out);

#endif
