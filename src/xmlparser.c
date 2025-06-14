#include "_libxml.h"

static xmlParserAttributes defaultAttribs = {
  .allocator = NULL,
  .srcConverter = &utf8Converter,
  .dstConverter = &utf8Converter,
};

xmlError
xmlInitParser (xmlParserAttributes *attribs, xmlParser **out)
{
  xmlBoolean hasStdAllocator = LXML_FALSE;
  xmlParser *parser;
  xmlAllocator *allocator;
  if (out == NULL)
    return LXML_ERR_ARG;
  if (attribs == NULL)
    attribs = &defaultAttribs;
  allocator = attribs->allocator;
  if (allocator == NULL)
    {
      hasStdAllocator = LXML_TRUE;
      XML_RETURN_ERROR (xmlInitStdAllocator (&allocator));
    }
  else
    allocator = attribs->allocator;
  parser = allocator->malloc (sizeof (xmlParser), allocator->ctx);
  if (parser == NULL)
    {
      if (hasStdAllocator == LXML_TRUE)
        XML_RETURN_ERROR (xmlDestroyStdAllocator (allocator));
      return LXML_ERR_ALLOC;
    }
  memset (parser, 0, sizeof (xmlParser));
  parser->hasStdAllocator = hasStdAllocator;
  parser->allocator = allocator;
  parser->srcConverter = attribs->srcConverter;
  if (parser->srcConverter == NULL)
    parser->srcConverter = &utf8Converter;
  parser->dstConverter = attribs->dstConverter;
  if (parser->dstConverter == NULL)
    parser->dstConverter = &utf8Converter;
  *out = parser;
  return LXML_ERR_NONE;
}

xmlError
xmlDestroyParser (xmlParser *parser)
{
  xmlBoolean hasStdAllocator;
  xmlAllocator *allocator;
  if (parser == NULL)
    return LXML_ERR_ARG;
  hasStdAllocator = parser->hasStdAllocator;
  allocator = parser->allocator;
  if (parser->scratch.buf != NULL)
    allocator->free (parser->scratch.buf, allocator->ctx);
  allocator->free (parser, allocator->ctx);
  if (hasStdAllocator == LXML_TRUE)
    XML_RETURN_ERROR (xmlDestroyStdAllocator (allocator));
  return LXML_ERR_NONE;
}
