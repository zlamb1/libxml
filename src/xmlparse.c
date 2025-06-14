#include "xmlparse.h"
#include "_libxml.h"

xmlError
xmlReadCharacter (xmlParser *parser)
{
  xmlScalarDecode decode;
  if (parser->pos >= parser->size)
    {
      switch (parser->parseMode)
        {
        case LXML_PARSE_MODE_STREAM:
          return LXML_ERR_LEN;
        case LXML_PARSE_MODE_DOM:
          parser->character = LXML_UC_NUL;
          parser->advance = 0;
          return LXML_ERR_NONE;
        }
    }
  decode = parser->srcConverter->decode (parser->src + parser->pos,
                                         parser->size - parser->pos);
  switch (decode.error)
    {
    case LXML_ERR_NONE:
      break;
    case LXML_ERR_LEN:
      parser->character = 0;
      return LXML_ERR_LEN;
    default:
      return decode.error;
    }
  switch (decode.scalar)
    {
    case LXML_UC_CR:
      parser->hadCR = LXML_TRUE;
      decode.scalar = LXML_UC_LF;
      break;
    case LXML_UC_LF:
      if (parser->hadCR == LXML_TRUE)
        {
          parser->hadCR = LXML_FALSE;
          parser->pos += parser->advance;
          parser->advance = 0;
          return xmlReadCharacter (parser);
        }
      break;
    default:
      parser->hadCR = LXML_FALSE;
      break;
    }
  parser->character = decode.scalar;
  parser->advance = decode.size;
  return LXML_ERR_NONE;
}

void
xmlConsumeCharacter (xmlParser *parser)
{
  parser->pos += parser->advance;
  parser->advance = 0;
}

xmlError
xmlParseTagType (xmlParser *parser)
{
  XML_RETURN_ERROR (xmlReadCharacter (parser));
  if (xmlIsNameStartCharacter (parser->character) == LXML_TRUE)
    {
      xmlNode *node;
      if (parser->root == NULL)
        {
          node = parser->allocator->malloc (sizeof (xmlNode),
                                            parser->allocator->ctx);
          if (node == NULL)
            return LXML_ERR_ALLOC;
          memset (node, 0, sizeof (xmlNode));
          parser->root = node;
        }
      else
        {
          xmlNode *parent = parser->node;
          if (parent == NULL)
            {
              printf ("unexpected tag outside of root element\n");
              return LXML_ERR_PARSE;
            }
          XML_RETURN_ERROR (
              xmlNodeAppendChild (parent, &node, parser->allocator));
        }
      node->type = LXML_NODE_TYPE_EMPTY;
      parser->node = node;
      XML_PUSH_COMMAND (parser, LXML_PARSE_STATE_TYPE_TAG_META);
      xmlConsumeWhiteSpace (parser);
      XML_PUSH_STRING_COMMAND (parser, LXML_PARSE_STATE_TYPE_NAME,
                               &node->name);
    }
  else if (parser->character == LXML_UC_SLASH)
    {
      xmlStringClear (&parser->scratch);
      xmlConsumeCharacter (parser);
      xmlPushParseCommand (
          parser, (xmlParseCommand){ .type = LXML_PARSE_STATE_TYPE_CHARACTER,
                                     .character = LXML_UC_GREATER_THAN,
                                     .onParse = xmlEndTag });
      xmlConsumeWhiteSpace (parser);
      XML_PUSH_STRING_COMMAND (parser, LXML_PARSE_STATE_TYPE_NAME,
                               &parser->scratch);
    }
  else if (parser->character == LXML_UC_EXCLAMATION)
    {
      xmlConsumeCharacter (parser);
      XML_PUSH_COMMAND (parser, LXML_PARSE_STATE_TYPE_COMMENT);
      xmlExpectCharacter (parser, LXML_UC_HYPHEN);
      xmlExpectCharacter (parser, LXML_UC_HYPHEN);
    }
  else
    {
      printf ("expected tag: got '%c'\n", (char) parser->character);
      return LXML_ERR_PARSE;
    }
  return LXML_ERR_NONE;
}

xmlError
xmlEndTag (xmlParser *parser, xmlParseCommand *command)
{
  xmlNode *node = parser->node;
  (void) command;
  if (node == NULL)
    {
      printf ("unexpected closing tag: </%s>\n", parser->scratch.buf);
      return LXML_ERR_PARSE;
    }
  if (parser->scratch.bytes != node->name.bytes
      || memcmp (parser->scratch.buf, node->name.buf, parser->scratch.bytes)
             != 0)
    {
      printf ("expected closing tag for <%s>: got </%s>\n", node->name.buf,
              parser->scratch.buf);
      return LXML_ERR_PARSE;
    }
  parser->node = node->parent;
  return LXML_ERR_NONE;
}
