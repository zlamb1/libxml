#include <assert.h>

#include "_libxml.h"
#include "xmlparser.h"

void
xmlPushParseCommand (xmlParser *parser, xmlParseCommand command)
{
  assert (parser->stackTop < LXML_PARSER_COMMAND_STACK_SIZE - 1);
  parser->commandStack[++parser->stackTop] = command;
}

void
xmlExpectCharacter (xmlParser *parser, xmlUTF32 expected)
{
  xmlPushParseCommand (
      parser, (xmlParseCommand){ .type = LXML_PARSE_STATE_TYPE_CHARACTER,
                                 .character = expected });
}

void
xmlConsumeWhiteSpace (xmlParser *parser)
{
  xmlPushParseCommand (
      parser, (xmlParseCommand){ .type = LXML_PARSE_STATE_TYPE_WHITESPACE,
                                 .required = LXML_FALSE });
}

void
xmlExpectWhiteSpace (xmlParser *parser)
{
  xmlPushParseCommand (
      parser, (xmlParseCommand){ .type = LXML_PARSE_STATE_TYPE_WHITESPACE,
                                 .required = LXML_TRUE });
}

xmlError
xmlPopParseCommand (xmlParser *parser, xmlParseCommand *command)
{
  if (parser->stackTop == -1)
    return LXML_ERR_LEN;
  if (command != NULL)
    *command = parser->commandStack[parser->stackTop];
  parser->stackTop--;
  return LXML_ERR_NONE;
}
