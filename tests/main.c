#include "xmlalloc.h"
#include "xmlparser.h"
#include "xmlstring.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "libxml.h"

static char *
readfile (const char *name, size_t *sz)
{
  FILE *fp;
  char *buf;
  long size, read;
  fp = fopen (name, "rb");
  if (fp == NULL)
    return NULL;
  if (fseek (fp, 0L, SEEK_END) != 0 || (size = ftell (fp)) == -1L
      || fseek (fp, 0L, SEEK_SET) != 0)
    goto fail;
  buf = malloc (size + 4);
  if (buf == NULL)
    goto fail;
  memset (buf + size, 0, 4);
  if ((read = fread (buf, 1, size, fp)) != size)
    {
      free (buf);
      goto fail;
    }
  if (sz != NULL)
    *sz = size;
  fclose (fp);
  return buf;
fail:
  fclose (fp);
  return NULL;
}

static void
printstarttag (xmlNode *node)
{
  printf ("<%s", node->name.buf);
  if (node->nattribs > 0)
    {
      printf (" ");
      for (xmlSize i = 0; i < node->nattribs; i++)
        {
          xmlNodeAttribute *attrib = node->attribs + i;
          printf ("%s=\"%s\"", attrib->name.buf, attrib->value.buf);
          if (i != node->nattribs - 1)
            printf (" ");
        }
    }
  printf (">");
}

static void
printendtag (xmlNode *node)
{
  printf ("</%s>", node->name.buf);
}

static void
printnode (xmlNode *node)
{
  switch (node->type)
    {
    case LXML_NODE_TYPE_EMPTY:
      printstarttag (node);
      printendtag (node);
      break;
    case LXML_NODE_TYPE_MIXED:
      printstarttag (node);
      for (xmlSize i = 0; i < node->nchildren; i++)
        {
          xmlNode *child = node->children + i;
          printnode (child);
        }
      printendtag (node);
      break;
    case LXML_NODE_TYPE_TEXT:
      printf ("%s", node->text.buf);
      break;
    default:
      break;
    }
}

static const char *EXAMPLE_FILES[1] = { "./tests/example01.xml" };

int
main (void)
{
  xmlAllocator *allocator;
  xmlParser *parser;
  xmlParserAttributes attribs = { 0 };
  char *buf;
  size_t len;
#ifdef _WIN32
  SetConsoleOutputCP (CP_UTF8);
#endif
  if (xmlInitStdDebugAllocator (&allocator) != LXML_ERR_NONE)
    {
      printf ("failed to init allocator\n");
      return -1;
    }
  attribs.allocator = allocator;
  if (xmlInitParser (&attribs, &parser) != LXML_ERR_NONE)
    {
      printf ("failed to init parser\n");
      return -1;
    }
  for (size_t i = 0; i < sizeof (EXAMPLE_FILES) / sizeof (const char *); i++)
    {
      xmlDocument *document;
      xmlError error = LXML_ERR_NONE;
      printf ("reading %s...\n", EXAMPLE_FILES[i]);
      buf = readfile (EXAMPLE_FILES[i], &len);
      if (buf == NULL)
        {
          printf ("failed to read: %s\n", EXAMPLE_FILES[i]);
          continue;
        }
      error = xmlParseDocument (buf, len, parser, &document);
      if (error == LXML_ERR_NONE)
        {
          xmlNode *root;
          if ((error = xmlGetDocumentRoot (document, &root)) != LXML_ERR_NONE)
            {
              printf ("failed to get root node -> %i\n", error);
            }
          else
            {
              printnode (root);
              printf ("\n");
            }
        }
      else
        {
          printf ("failed to parse document -> %i\n", error);
        }
      xmlDestroyDocument (document);
      free (buf);
    }
  if (xmlDestroyParser (parser) != LXML_ERR_NONE)
    {
      printf ("failed to destroy parser\n");
      return -1;
    }
  {
    xmlDebugMetrics metrics;
    xmlDebugAllocatorGetMetrics (allocator, &metrics);
    printf ("bytes allocated: %zu\n", metrics.bytesAllocated);
    printf ("bytes freed: %zu\n", metrics.bytesFreed);
  }
  xmlDestroyDebugAllocator (allocator);
  return 0;
}