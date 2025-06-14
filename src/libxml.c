#include "libxml.h"
#include "_libxml.h"
#include "xmlalloc.h"
#include "xmlencoding.h"
#include "xmlparse.h"
#include "xmlparser.h"
#include "xmlstring.h"
#include "xmltype.h"
#include "xmlunicode.h"
#include <string.h>

static inline xmlError
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

static inline void
xmlConsumeCharacter (xmlParser *parser)
{
  parser->pos += parser->advance;
  parser->advance = 0;
}

static inline xmlError
xmlAppendChild (xmlNode *parent, xmlNode **out, xmlAllocator *allocator)
{
  xmlNode *node, *c;
  switch (parent->type)
    {
    case LXML_NODE_TYPE_EMPTY:
      parent->type = LXML_NODE_TYPE_MIXED;
      parent->children = NULL;
      parent->nchildren = 0;
      parent->cchildren = 0;
    case LXML_NODE_TYPE_MIXED:
      break;
    default:
      printf ("invalid node type for children: %i\n", parent->type);
      return LXML_ERR_PARSE;
    }
  c = parent->children;
  XML_MAYBE_EXPAND (parent->children, xmlNode, parent->nchildren,
                    parent->cchildren, allocator);
  if (c != parent->children)
    {
      for (xmlSize i = 0; i < parent->nchildren; i++)
        {
          /* update child pointers to moved parents */
          xmlNode *child = parent->children + i;
          if (child->type == LXML_NODE_TYPE_MIXED)
            {
              for (xmlSize j = 0; j < child->nchildren; j++)
                (child->children + j)->parent = child;
            }
        }
    }
  node = parent->children + parent->nchildren++;
  memset (node, 0, sizeof (xmlNode));
  node->parent = parent;
  *out = node;
  return LXML_ERR_NONE;
}

static xmlError
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

static xmlError
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
          XML_RETURN_ERROR (xmlAppendChild (parent, &node, parser->allocator));
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

static xmlError
xmlProcessCommands (xmlParser *parser)
{
  xmlParseCommand command;
  while (xmlPopParseCommand (parser, &command) == LXML_ERR_NONE)
    {
      switch (command.type)
        {
        case LXML_PARSE_STATE_TYPE_CHARACTER:
          XML_RETURN_ERROR (xmlReadCharacter (parser));
          if (parser->character != command.character)
            {
              printf ("expected '%c'; got '%c'\n", (char) command.character,
                      (char) parser->character);
              return LXML_ERR_PARSE;
            }
          xmlConsumeCharacter (parser);
          break;
        case LXML_PARSE_STATE_TYPE_WHITESPACE:
          XML_RETURN_ERROR (xmlReadCharacter (parser));
          while (xmlIsWhiteSpace (parser->character) == LXML_TRUE)
            {
              xmlConsumeCharacter (parser);
              XML_RETURN_ERROR (xmlReadCharacter (parser));
            }
          break;
        case LXML_PARSE_STATE_TYPE_QUOTES:
          XML_RETURN_ERROR (xmlReadCharacter (parser));
          if (parser->character != LXML_UC_QUOTE
              && parser->character != LXML_UC_QUOTE)
            {
              printf ("expected '\"' character\n");
              return LXML_ERR_PARSE;
            }
          break;
        case LXML_PARSE_STATE_TYPE_NAME:
          xmlStringClear (&parser->scratch);
          XML_RETURN_ERROR (xmlReadCharacter (parser));
          while (xmlIsNameCharacter (parser->character)) // 40402902
            {
              XML_RETURN_ERROR (xmlStringAppend (
                  &parser->scratch, parser->allocator, parser->dstConverter,
                  parser->character, LXML_TRUE));
              xmlConsumeCharacter (parser);
              XML_RETURN_ERROR (xmlReadCharacter (parser));
            }
          xmlStringTerminate (&parser->scratch, parser->allocator,
                              parser->dstConverter);
          XML_RETURN_ERROR (xmlStringDuplicate (
              &parser->scratch, parser->allocator, command.string));
          break;
        case LXML_PARSE_STATE_TYPE_TAG_OR_CONTENT:
          XML_RETURN_ERROR (xmlReadCharacter (parser));
          switch (parser->character)
            {
            case LXML_UC_LESS_THAN:
              xmlConsumeCharacter (parser);
              XML_PUSH_COMMAND (parser, LXML_PARSE_STATE_TYPE_TAG_TYPE);
              break;
            case LXML_UC_AMPERSAND:
              // TODO: handle this
              printf ("unexpected '&' in document\n");
              return LXML_ERR_PARSE;
            default:
              {
                // TODO: handle ]]>
                xmlNode *parent = parser->node, *node;
                if (parent == NULL)
                  {
                    printf ("unexpected content outside root tag\n");
                    return LXML_ERR_PARSE;
                  }
                XML_RETURN_ERROR (
                    xmlAppendChild (parent, &node, parser->allocator));
                node->type = LXML_NODE_TYPE_TEXT;
                XML_PUSH_STRING_COMMAND (parser, LXML_PARSE_STATE_TYPE_CONTENT,
                                         &node->text);
                break;
              }
            }
          break;
        case LXML_PARSE_STATE_TYPE_TAG_TYPE:
          XML_RETURN_ERROR (xmlParseTagType (parser));
          break;
        case LXML_PARSE_STATE_TYPE_TAG_META:
          XML_RETURN_ERROR (xmlReadCharacter (parser));
          if (parser->character == LXML_UC_GREATER_THAN)
            {
              xmlConsumeCharacter (parser);
              break;
            }
          else if (parser->character == LXML_UC_SLASH)
            {
              xmlConsumeCharacter (parser);
              xmlExpectCharacter (parser, LXML_UC_GREATER_THAN);
              parser->node = parser->node->parent;
              break;
            }
          else if (xmlIsNameStartCharacter (parser->character))
            {
              xmlNode *node = parser->node;
              xmlNodeAttribute *attrib;
              XML_MAYBE_EXPAND (node->attribs, xmlNodeAttribute,
                                node->nattribs, node->cattribs,
                                parser->allocator);
              attrib = node->attribs + node->nattribs++;
              memset (attrib, 0, sizeof (xmlNodeAttribute));
              XML_PUSH_COMMAND (parser, LXML_PARSE_STATE_TYPE_TAG_META);
              xmlConsumeWhiteSpace (parser);
              XML_PUSH_STRING_COMMAND (
                  parser, LXML_PARSE_STATE_TYPE_ATTRIB_VALUE, &attrib->value);
              XML_PUSH_COMMAND (parser, LXML_PARSE_STATE_TYPE_QUOTES);
              xmlConsumeWhiteSpace (parser);
              xmlExpectCharacter (parser, LXML_UC_EQUAL);
              xmlConsumeWhiteSpace (parser);
              XML_PUSH_STRING_COMMAND (parser, LXML_PARSE_STATE_TYPE_NAME,
                                       &attrib->name);
              break;
            }
          else
            {
              printf ("expected '>' or attribute: got '%c'\n",
                      (char) parser->character);
              return LXML_ERR_PARSE;
            }
          break;
        case LXML_PARSE_STATE_TYPE_ATTRIB_VALUE:
          {
            xmlUTF32 endCharacter = parser->character;
            xmlStringClear (&parser->scratch);
            xmlConsumeCharacter (parser);
            XML_RETURN_ERROR (xmlReadCharacter (parser));
            // TODO: handle entity references + normalization
            while (parser->character != endCharacter)
              {
                switch (parser->character)
                  {
                  case LXML_UC_LESS_THAN:
                    printf ("unexpected '<' inside attribute\n");
                    return LXML_ERR_PARSE;
                  case LXML_UC_AMPERSAND:
                    // TODO: handle this
                    printf ("unexpected '&' inside attribute\n");
                    return LXML_ERR_PARSE;
                  default:
                    break;
                  }
                XML_RETURN_ERROR (xmlStringAppend (
                    &parser->scratch, parser->allocator, parser->dstConverter,
                    parser->character, LXML_TRUE));
                xmlConsumeCharacter (parser);
                XML_RETURN_ERROR (xmlReadCharacter (parser));
              }
            xmlConsumeCharacter (parser);
            xmlStringTerminate (&parser->scratch, parser->allocator,
                                parser->dstConverter);
            xmlStringDuplicate (&parser->scratch, parser->allocator,
                                command.string);
            break;
          }
        case LXML_PARSE_STATE_TYPE_CONTENT:
          xmlStringClear (&parser->scratch);
          XML_RETURN_ERROR (xmlReadCharacter (parser));
          // TODO: handle ]]>
          while (parser->character != LXML_UC_NUL)
            {
              switch (parser->character)
                {
                case LXML_UC_LESS_THAN:
                case LXML_UC_AMPERSAND:
                  goto out;
                default:
                  break;
                }
              XML_RETURN_ERROR (xmlStringAppend (
                  &parser->scratch, parser->allocator, parser->dstConverter,
                  parser->character, LXML_TRUE));
              xmlConsumeCharacter (parser);
              XML_RETURN_ERROR (xmlReadCharacter (parser));
            }
        out:
          xmlStringTerminate (&parser->scratch, parser->allocator,
                              parser->dstConverter);
          xmlStringDuplicate (&parser->scratch, parser->allocator,
                              command.string);
          break;
        case LXML_PARSE_STATE_TYPE_COMMENT:
          {
            xmlBoolean hadHyphen = LXML_FALSE;
            XML_RETURN_ERROR (xmlReadCharacter (parser));
            while (xmlIsCharacter (parser->character))
              {
                if (parser->character == LXML_UC_HYPHEN)
                  {
                    if (hadHyphen == LXML_TRUE)
                      {
                        xmlConsumeCharacter (parser);
                        xmlExpectCharacter (parser, LXML_UC_GREATER_THAN);
                        goto out2;
                      }
                    hadHyphen = LXML_TRUE;
                  }
                else
                  hadHyphen = LXML_FALSE;
                xmlConsumeCharacter (parser);
                XML_RETURN_ERROR (xmlReadCharacter (parser));
              }
            printf ("unexpected character in comment: '%c'\n",
                    (char) parser->character);
            return LXML_ERR_PARSE;
          out2:
            break;
          }
        default:
          printf ("unexpected command: %i\n", command.type);
          return LXML_ERR_PARSE;
        }
      if (command.onParse != NULL)
        XML_RETURN_ERROR (command.onParse (parser, &command));
    }
  return LXML_ERR_NONE;
}

xmlError
xmlParseDocument (xmlChar *src, xmlSize size, xmlParser *parser,
                  xmlDocument **out)
{
  xmlDocument *document;
  xmlError error = LXML_ERR_NONE;
  if (src == NULL || size == 0 || parser == NULL || out == NULL)
    return LXML_ERR_ARG;
  document = parser->allocator->malloc (sizeof (xmlDocument),
                                        parser->allocator->ctx);
  if (document == NULL)
    return LXML_ERR_PARSE;
  memset (document, 0, sizeof (xmlDocument));
  document->version = LXML_VERSION_1_0;
  document->allocator = parser->allocator;
  *out = document;
  parser->parseMode = LXML_PARSE_MODE_DOM;
  parser->pos = 0;
  parser->size = size;
  parser->src = (xmlUChar *) src;
  parser->stackTop = -1;
  xmlStringClear (&parser->scratch);
  while (parser->pos < parser->size)
    {
      XML_PUSH_COMMAND (parser, LXML_PARSE_STATE_TYPE_TAG_OR_CONTENT);
      XML_CHECK_ERROR (xmlProcessCommands (parser), error, fail);
    }
  if (parser->node != NULL)
    {
      printf ("unclosed tag: %s\n", parser->node->name.buf);
      return LXML_ERR_PARSE;
    }
  document->root = parser->root;
  return LXML_ERR_NONE;
fail:
  return error;
}

xmlError
xmlGetDocumentRoot (xmlDocument *document, xmlNode **out)
{
  if (document == NULL || out == NULL)
    return LXML_ERR_ARG;
  *out = document->root;
  return LXML_ERR_NONE;
}

xmlError
xmlGetDocumentStringIterator (xmlDocument *document, xmlString *string,
                              xmlStringIterator *iterator)
{
  if (document == NULL || iterator == NULL)
    return LXML_ERR_ARG;
  *iterator = (xmlStringIterator){ .pos = 0, .string = string };
  return LXML_ERR_NONE;
}

xmlError
xmlGetTreeMemorySize (xmlNode *root, xmlSize *out)
{
  long top = -1, cap = 64;
  xmlNode **stk;
  xmlSize bytes = 0;
  if (root == NULL || out == NULL)
    return LXML_ERR_ARG;
  stk = malloc (sizeof (xmlNode **) * cap);
  if (stk == NULL)
    return LXML_ERR_ALLOC;
  stk[++top] = root;
  while (top > -1)
    {
      xmlNode *node = stk[top--];
      switch (node->type)
        {
        case LXML_NODE_TYPE_MIXED:
          bytes += node->cchildren * sizeof (xmlNode);
          for (xmlSize i = 0; i < node->nchildren; i++)
            {
              xmlNode *child = node->children + i;
              if (top >= cap - 1)
                {
                  xmlNode **newstk;
                  while (top >= cap - 1)
                    cap *= 2;
                  newstk = realloc (stk, sizeof (xmlNode **) * cap);
                  if (newstk == NULL)
                    {
                      free (stk);
                      return LXML_ERR_ALLOC;
                    }
                  stk = newstk;
                }
              stk[++top] = child;
            }
          // fallthrough
        case LXML_NODE_TYPE_EMPTY:
          bytes += node->name.cap;
          bytes += node->cattribs * sizeof (xmlNodeAttribute);
          for (xmlSize i = 0; i < node->nattribs; i++)
            {
              xmlNodeAttribute *attrib = node->attribs + i;
              bytes += attrib->name.cap;
              bytes += attrib->value.cap;
            }
          break;
        case LXML_NODE_TYPE_TEXT:
          bytes += node->text.cap;
          break;
        case LXML_NODE_TYPE_COMMENT:
          bytes += node->comment.cap;
          break;
        }
    }
  *out = bytes;
  free (stk);
  return LXML_ERR_NONE;
}

xmlError
xmlDestroyNode (xmlNode *root, xmlAllocator *allocator, xmlBoolean destroyRoot)
{
  xmlNode *node = root;
  if (root == NULL || allocator == NULL)
    return LXML_ERR_ARG;
  while (node != NULL)
    {
      switch (node->type)
        {
        case LXML_NODE_TYPE_MIXED:
          if (node->nchildren)
            {
              node->nchildren--;
              node = node->children + node->nchildren;
              break;
            }
          if (node->children != NULL)
            {
              allocator->free (node->children, allocator->ctx);
              node->children = NULL;
            }
          // fallthrough
        case LXML_NODE_TYPE_EMPTY:
          xmlDestroyString (&node->name, allocator);
          if (node->attribs != NULL)
            {
              for (xmlSize i = 0; i < node->nattribs; i++)
                {
                  xmlNodeAttribute *attrib = node->attribs + i;
                  if (attrib->name.buf != NULL)
                    xmlDestroyString (&attrib->name, allocator);
                  if (attrib->value.buf != NULL)
                    xmlDestroyString (&attrib->value, allocator);
                }
              allocator->free (node->attribs, allocator->ctx);
              node->attribs = NULL;
            }
          node = node->parent;
          break;
        case LXML_NODE_TYPE_TEXT:
          if (node->text.buf != NULL)
            xmlDestroyString (&node->text, allocator);
          node = node->parent;
          break;
        case LXML_NODE_TYPE_COMMENT:
          if (node->comment.buf != NULL)
            xmlDestroyString (&node->comment, allocator);
          node = node->parent;
          break;
        }
    }
  if (destroyRoot == LXML_TRUE)
    allocator->free (root, allocator->ctx);
  return LXML_ERR_NONE;
}

xmlError
xmlDestroyDocument (xmlDocument *document)
{
  xmlAllocator *allocator;
  xmlNode *root;
  if (document == NULL)
    return LXML_ERR_ARG;
  allocator = document->allocator;
  root = document->root;
  if (root != NULL)
    {
      XML_RETURN_ERROR (xmlDestroyNode (root, allocator, LXML_TRUE));
    }
  allocator->free (document, allocator->ctx);
  return LXML_ERR_NONE;
}
