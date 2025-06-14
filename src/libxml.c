#include <string.h>

#include "_libxml.h"

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
          {
            xmlSize occurences = 0;
            XML_RETURN_ERROR (xmlReadCharacter (parser));
            while (xmlIsWhiteSpace (parser->character) == LXML_TRUE)
              {
                occurences++;
                xmlConsumeCharacter (parser);
                XML_RETURN_ERROR (xmlReadCharacter (parser));
              }
            if (occurences == 0 && command.required == LXML_TRUE)
              {
                printf ("expected ' '; got '%c'\n", (char) parser->character);
                return LXML_ERR_PARSE;
              }
            break;
          }
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
                    xmlNodeAppendChild (parent, &node, parser->allocator));
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
