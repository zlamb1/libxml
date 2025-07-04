#include <string.h>

#include "xmlalloc.h"
#include "xmlencoding.h"
#include "xmlstring.h"
#include "xmltype.h"

xmlError
xmlStringAppend (xmlString *string, xmlAllocator *allocator,
                 xmlEncodingConverter *encoding, xmlUTF32 character,
                 xmlBoolean setlen)
{
  xmlScalarEncode encode;
  xmlSize size, cap;
  if (string == NULL || allocator == NULL || encoding == NULL
      || string->bytes > string->cap)
    return LXML_ERR_ARG;
  encode = encoding->encode (character,
                             (xmlUChar *) (string->buf + string->bytes),
                             string->cap - string->bytes);
  switch (encode.error)
    {
    case LXML_ERR_NONE:
      if (setlen)
        string->len++;
      string->bytes += encode.size;
      return LXML_ERR_NONE;
    case LXML_ERR_LEN:
      break;
    default:
      return encode.error;
    }
  size = string->bytes + encode.size;
  cap = string->cap;
  if (cap == 0)
    string->cap = 1;
  while (string->cap < size)
    string->cap *= 2;
  if (cap != string->cap)
    {
      xmlChar *buf
          = allocator->realloc (string->buf, string->cap, allocator->ctx);
      if (buf == NULL)
        return LXML_ERR_ALLOC;
      string->buf = buf;
    }
  encode = encoding->encode (character,
                             (xmlUChar *) (string->buf + string->bytes),
                             string->cap - string->bytes);
  if (encode.error != LXML_ERR_NONE)
    return encode.error;
  if (setlen)
    string->len++;
  string->bytes += encode.size;
  return LXML_ERR_NONE;
}

xmlError
xmlStringDuplicate (xmlString *string, xmlAllocator *allocator, xmlString *out)
{
  if (string == NULL || allocator == NULL || out == NULL)
    return LXML_ERR_ARG;
  if (string == out)
    return LXML_ERR_NONE;
  if (string->bytes == 0)
    {
      out->len = 0;
      out->bytes = 0;
      out->cap = 0;
      out->buf = NULL;
      return LXML_ERR_NONE;
    }
  out->buf = allocator->malloc (string->bytes, allocator->ctx);
  if (out->buf == NULL)
    return LXML_ERR_ALLOC;
  memcpy (out->buf, string->buf, string->bytes);
  out->len = string->len;
  out->bytes = string->bytes;
  out->cap = string->bytes;
  return LXML_ERR_NONE;
}

xmlError
xmlStringClear (xmlString *string)
{
  if (string == NULL)
    return LXML_ERR_ARG;
  string->bytes = 0;
  return LXML_ERR_NONE;
}

xmlError
xmlDestroyString (xmlString *string, xmlAllocator *allocator)
{
  if (string == NULL || allocator == NULL)
    return LXML_ERR_ARG;
  if (string->buf != NULL)
    {
      allocator->free (string->buf, allocator->ctx);
      string->buf = NULL;
    }
  return LXML_ERR_NONE;
}

xmlError
xmlStringIteratorNext (xmlStringIterator *iterator, xmlUTF32 *out)
{
  xmlScalarDecode decode;
  if (iterator == NULL)
    return LXML_ERR_ARG;
  if (iterator->pos >= iterator->string->bytes)
    return LXML_ERR_LEN;
  decode
      = iterator->encoding->decode ((xmlUChar *) iterator->string->buf,
                                    iterator->string->bytes - iterator->pos);
  XML_RETURN_ERROR (decode.error);
  iterator->pos += decode.size;
  if (out != NULL)
    *out = decode.scalar;
  return LXML_ERR_NONE;
}
