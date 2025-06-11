#ifndef _LIBXML_H
#define _LIBXML_H

/* internal header */

#include <string.h>
#include <stdio.h>

#include "libxml.h"
#include "xmlalloc.h"
#include "xmlparse.h"

typedef struct
xmlBuffer
{
    xmlSize cnt, cap;
    xmlByte *buf;
} xmlBuffer;

typedef enum
xmlParserCommandType 
{
    LXML_PARSER_STATE_TYPE_CHARACTER,
    LXML_PARSER_STATE_TYPE_NAME,
    LXML_PARSER_STATE_TYPE_TAG_TYPE,
} xmlParserCommandType;

typedef struct
xmlParseCommand
{
    xmlParserCommandType type;
    xmlError (*onParse)(struct xmlParser *parser, struct xmlParseCommand *state);
    union {
        xmlUTF32 character;
        xmlString *string;
    };
} xmlParseCommand;

#define LXML_PARSER_COMMAND_QUEUE_SIZE 64

typedef struct 
xmlParser 
{
    xmlBoolean isStdAllocator;
    xmlAllocator *allocator;
    xmlEncodingConverter *srcConverter, *dstConverter;
    xmlSize pos, size;
    xmlUChar *src;
    xmlSize advance;
    xmlUTF32 character;
    xmlSize head, tail;
    xmlParseCommand queue[LXML_PARSER_COMMAND_QUEUE_SIZE];
} xmlParser;

typedef struct
xmlDocument
{
    xmlVersion version;
    xmlAllocator *allocator;
    xmlEncodingConverter *dstConverter;
    xmlNode *root, *node;
    xmlString error; 
    xmlString scratch;
    xmlBuffer overflow;
} xmlDocument;

void
xmlQueueParseCommand(xmlParser *parser, xmlParseCommand command);

void
xmlExpectCharacter(xmlParser *parser, xmlUTF32 character);

xmlError
xmlDequeueParseCommand(xmlParser *parser, xmlParseCommand *command);

#endif
