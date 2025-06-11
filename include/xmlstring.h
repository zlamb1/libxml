#ifndef XMLSTRING_H
#define XMLSTRING_H 1

#include "xmlalloc.h"
#include "xmltype.h"
#include "xmlencoding.h"

typedef struct
xmlString
{
    xmlSize bytes, cap;
    xmlChar *buf;
} xmlString; 

typedef struct
xmlStringIterator
{
    xmlEncodingConverter *encoding;
    xmlSize pos;
    xmlString *string;
} xmlStringIterator;

xmlError
xmlStringAppendCharacter(xmlString *string, xmlAllocator *allocator, xmlEncodingConverter *dstConverter, xmlUTF32 character);

xmlError
xmlStringAppendString(xmlString *string, xmlAllocator *allocator, xmlEncodingConverter *dstConverter, const xmlChar *str); 

xmlError
xmlDestroyString(xmlString *string, xmlAllocator *allocator);

xmlError 
xmlStringIteratorHasNext(xmlStringIterator *iterator);

xmlUTF32
xmlStringIteratorNext(xmlStringIterator *iterator);

#endif
