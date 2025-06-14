#ifndef XMLENCODING_H
#define XMLENCODING_H 1

#include "xmltype.h"

typedef unsigned long xmlUTF32;

typedef struct xmlScalarEncode
{
  xmlError error;
  xmlSize size;
} xmlScalarEncode;

typedef struct xmlScalarDecode
{
  xmlError error;
  xmlUTF32 scalar;
  xmlSize size;
} xmlScalarDecode;

typedef struct xmlEncodingConverter
{
  xmlScalarEncode (*encode) (xmlUTF32 scalar, xmlUChar *dst, xmlSize len);
  xmlScalarDecode (*decode) (xmlUChar *src, xmlSize len);
} xmlEncodingConverter;

extern xmlEncodingConverter utf8Converter;

#endif
