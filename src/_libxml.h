#ifndef _LIBXML_H
#define _LIBXML_H

/* internal header */

#include <stdio.h>
#include <string.h>

#include "libxml.h"
#include "xmlalloc.h"
#include "xmlparse.h"
#include "xmltype.h"

typedef struct xmlBuffer
{
  xmlSize cnt, cap;
  xmlByte *buf;
} xmlBuffer;

typedef enum xmlParserCommandType
{
  LXML_PARSE_STATE_TYPE_CHARACTER,
  LXML_PARSE_STATE_TYPE_WHITESPACE,
  LXML_PARSE_STATE_TYPE_QUOTES,
  LXML_PARSE_STATE_TYPE_NAME,
  LXML_PARSE_STATE_TYPE_TAG_OR_CONTENT,
  LXML_PARSE_STATE_TYPE_TAG_TYPE,
  LXML_PARSE_STATE_TYPE_TAG_META,
  LXML_PARSE_STATE_TYPE_ATTRIB_VALUE,
  LXML_PARSE_STATE_TYPE_CONTENT,
  LXML_PARSE_STATE_TYPE_COMMENT
} xmlParserCommandType;

typedef struct xmlParseCommand
{
  xmlParserCommandType type;
  xmlError (*onParse) (struct xmlParser *parser,
                       struct xmlParseCommand *state);
  union
  {
    xmlUTF32 character;
    xmlString *string;
  };
} xmlParseCommand;

#define LXML_PARSER_COMMAND_STACK_SIZE 64

typedef enum xmlParseMode
{
  LXML_PARSE_MODE_STREAM,
  LXML_PARSE_MODE_DOM
} xmlParseMode;

typedef struct xmlParser
{
  xmlBoolean hasStdAllocator;
  xmlAllocator *allocator;
  xmlEncodingConverter *srcConverter, *dstConverter;
  xmlParseMode parseMode;
  xmlSize pos, size;
  xmlUChar *src;
  xmlBoolean hadCR;
  xmlSize advance;
  xmlUTF32 character;
  long stackTop;
  xmlParseCommand commandStack[LXML_PARSER_COMMAND_STACK_SIZE];
  xmlNode *root;
  xmlNode *node;
  xmlString scratch;
} xmlParser;

typedef struct xmlDocument
{
  xmlVersion version;
  xmlAllocator *allocator;
  xmlEncodingConverter *dstConverter;
  xmlNode *root, *node;
  xmlString error;
  xmlBuffer overflow;
} xmlDocument;

#define XML_PUSH_COMMAND(PARSER, TYPE)                                        \
  xmlPushParseCommand ((PARSER), (xmlParseCommand){ .type = (TYPE) });

#define XML_PUSH_STRING_COMMAND(PARSER, TYPE, STRING)                         \
  xmlPushParseCommand (                                                       \
      (PARSER), (xmlParseCommand){ .type = (TYPE), .string = (STRING) });

void xmlPushParseCommand (xmlParser *parser, xmlParseCommand command);

void xmlExpectCharacter (xmlParser *parser, xmlUTF32 character);

void xmlConsumeWhiteSpace (xmlParser *parser);

xmlError xmlPopParseCommand (xmlParser *parser, xmlParseCommand *command);

#endif
