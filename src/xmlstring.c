#include "xmlstring.h"
#include "xmlunicode.h"

xmlError
xmlStringAppendCharacter(xmlString *string, xmlAllocator *allocator, xmlEncodingConverter *dstConverter, xmlUTF32 character)
{
    xmlScalarEncode encode;
    if ( string == NULL || allocator == NULL || dstConverter == NULL )
        return LXML_ERR_ARG;
    encode = dstConverter->encode(character, NULL, 0); 
    if ( encode.error != LXML_ERR_LEN )
        return encode.error;
    return LXML_ERR_NONE;
}

xmlError
xmlStringAppendString(xmlString *string, const xmlChar *str)
{
    return LXML_ERR_NONE;
}

xmlError
xmlDestroyString(xmlString *string)
{
    return LXML_ERR_NONE;
}

xmlError 
xmlStringIteratorHasNext(xmlStringIterator *iterator)
{
    return LXML_ERR_NONE;
}

xmlUTF32
xmlStringIteratorNext(xmlStringIterator *iterator)
{
    return 0;
}