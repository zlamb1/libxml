#ifndef XMLTYPE_H
#define XMLTYPE_H 1

#include <stdint.h>
#include <stdlib.h>

#define LXML_FALSE 0
#define LXML_TRUE  1

#define XML_CHECK_ERROR(FUNC, ERROR, LABEL)                                   \
  do                                                                          \
    {                                                                         \
      if (((ERROR) = (FUNC)) != LXML_ERR_NONE)                                \
        goto LABEL;                                                           \
    }                                                                         \
  while (0)

#define XML_RETURN_ERROR(FUNC)                                                \
  do                                                                          \
    {                                                                         \
      xmlError error;                                                         \
      if ((error = (FUNC)) != LXML_ERR_NONE)                                  \
        return error;                                                         \
    }                                                                         \
  while (0)

typedef enum xmlError
{
  LXML_ERR_NONE = 0,
  LXML_ERR_PARSE = -1,
  LXML_ERR_ARG = -2,
  LXML_ERR_LEN = -3,
  LXML_ERR_ALLOC = -4,
  LXML_ERR_SURROGATE = -5,
  LXML_ERR_OVERLONG = -6,
  LXML_ERR_DECODE = -7,
  LXML_ERR_ENCODE = -8
} xmlError;

typedef intptr_t xmlIntPtr;
typedef uintptr_t xmlUIntPtr;
typedef unsigned char xmlByte;
typedef unsigned char xmlUChar;
typedef char xmlChar;
typedef int xmlBoolean;
typedef size_t xmlSize;

#endif
