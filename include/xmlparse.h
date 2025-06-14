#ifndef XMLPARSE_H
#define XMLPARSE_H 1

#include "xmlencoding.h"
#include "xmlparser.h"
#include "xmltype.h"
#include "xmlunicode.h"

static inline xmlBoolean
xmlIsAlpha (xmlUTF32 scalar)
{
  return (scalar >= 0x41 && scalar <= 0x5A)
         || (scalar >= 0x61 && scalar <= 0x7A);
}

static inline xmlBoolean
xmlIsNumeric (xmlUTF32 scalar)
{
  return scalar >= 0x30 && scalar <= 0x39;
}

static inline xmlBoolean
xmlIsAlphaNumeric (xmlUTF32 scalar)
{
  return xmlIsAlpha (scalar) || xmlIsNumeric (scalar);
}

static inline xmlBoolean
xmlIsWhiteSpace (xmlUTF32 scalar)
{
  switch (scalar)
    {
    case LXML_UC_TAB:
    case LXML_UC_LF:
    case LXML_UC_CR:
    case LXML_UC_SPACE:
      return LXML_TRUE;
    default:
      return LXML_FALSE;
    }
}

static inline xmlBoolean
xmlIsCharacter (xmlUTF32 character)
{
  switch (character)
    {
    case LXML_UC_TAB:
    case LXML_UC_CR:
    case LXML_UC_LF:
      return LXML_TRUE;
    default:
      return (character >= 0x20 && character <= 0xD7FF)
             || (character >= 0xE000 && character <= 0xFFFD)
             || (character >= 0x10000 && character <= 0x10FFFF);
    }
}

static inline xmlBoolean
xmlIsNameStartCharacter (xmlUTF32 scalar)
{
  switch (scalar)
    {
    case LXML_UC_COLON:
    case LXML_UC_UNDERSCORE:
      return LXML_TRUE;
    default:
      return xmlIsAlpha (scalar) || (scalar >= 0x000C0 && scalar <= 0x000D6)
             || (scalar >= 0x000D8 && scalar <= 0x000F6)
             || (scalar >= 0x000F8 && scalar <= 0x002FF)
             || (scalar >= 0x00370 && scalar <= 0x0037D)
             || (scalar >= 0x0037F && scalar <= 0x01FFF)
             || (scalar >= 0x0200C && scalar <= 0x0200D)
             || (scalar >= 0x02070 && scalar <= 0x0218F)
             || (scalar >= 0x02C00 && scalar <= 0x02FEF)
             || (scalar >= 0x03001 && scalar <= 0x0D7FF)
             || (scalar >= 0x0F900 && scalar <= 0x0FDCF)
             || (scalar >= 0x0FDF0 && scalar <= 0x0FFFD)
             || (scalar >= 0x10000 && scalar <= 0xEFFFF);
    }
}

static inline xmlBoolean
xmlIsNameCharacter (xmlUTF32 scalar)
{
  switch (scalar)
    {
    case LXML_UC_HYPHEN:
    case LXML_UC_PERIOD:
    case LXML_UC_MIDDLEDOT:
      return LXML_TRUE;
    default:
      return xmlIsNameStartCharacter (scalar) || xmlIsNumeric (scalar)
             || (scalar >= 0x0300 && scalar <= 0x036F)
             || (scalar >= 0x203F && scalar <= 0x2040);
    }
}

typedef struct xmlParseCommand xmlParseCommand;

xmlError xmlReadCharacter (xmlParser *parser);

void xmlConsumeCharacter (xmlParser *parser);

xmlError xmlParseTagType (xmlParser *parser);

xmlError xmlEndTag (xmlParser *parser, xmlParseCommand *command);

#endif
