#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "xmlalloc.h"

typedef struct xmlAlloc
{
  xmlBoolean isTombstone;
  xmlSize size;
  void *p;
} xmlAlloc;

typedef struct xmlAllocTable
{
  xmlSize cnt, cap;
  xmlAlloc *allocs;
} xmlAllocTable;

typedef struct xmlDebugAllocator
{
  xmlAllocator debug;
  xmlAllocator *allocator;
  xmlBoolean isOwning;
  xmlAllocTable table;
  xmlSize bytesAllocated;
  xmlSize bytesFreed;
} xmlDebugAllocator;
#include <stdio.h>

static void *
xmlMalloc (xmlSize size, void *ctx)
{
  (void) ctx;
  return malloc (size);
}

static void *
xmlRealloc (void *p, xmlSize size, void *ctx)
{
  (void) ctx;
  return realloc (p, size);
}

static void
xmlFree (void *p, void *ctx)
{
  (void) ctx;
  free (p);
}

static inline xmlUIntPtr
xmlHashAddress (xmlUIntPtr x)
{
  x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
  x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
  x = x ^ (x >> 31);
  return x;
}

static xmlAlloc *
xmlFindAlloc (xmlAllocTable *table, void *p)
{
  xmlAlloc *alloc;
  xmlUIntPtr idx;
  assert (table != NULL);
  assert (p != NULL);
  idx = xmlHashAddress ((xmlUIntPtr) p) % table->cap;
  alloc = table->allocs + idx;
  while (alloc->p != NULL || alloc->isTombstone == 1)
    {
      if (alloc->p == p)
        {
          assert (alloc->isTombstone == 0);
          return alloc;
        }
      idx = (idx + 1) % table->cap;
      alloc = table->allocs + idx;
    }
  assert (0);
  return NULL;
}

static void
xmlAppendAlloc (xmlAllocTable *table, void *p, xmlSize size)
{
  xmlAlloc *alloc;
  xmlUIntPtr idx;
  double load;
  assert (table != NULL);
  assert (p != NULL);
  if (table->allocs == NULL)
    {
      table->cnt = 0;
      table->cap = 1;
      table->allocs = malloc (sizeof (xmlAlloc));
      assert (table->allocs != NULL);
      memset (table->allocs, 0, sizeof (xmlAlloc));
    }
  load = table->cnt / (double) table->cap;
  if (load >= 0.75)
    {
      xmlAlloc *allocs;
      xmlSize cap = table->cap;
      xmlSize bytes;
      table->cap *= 2;
      bytes = sizeof (xmlAlloc) * table->cap;
      allocs = malloc (bytes);
      assert (allocs != NULL);
      memset (allocs, 0, bytes);
      for (xmlSize i = 0; i < cap; i++)
        {
          alloc = table->allocs + i;
          if (alloc->p != NULL)
            {
              xmlAlloc *newalloc;
              idx = xmlHashAddress ((xmlUIntPtr) alloc->p) % table->cap;
              newalloc = allocs + idx;
              while (newalloc->p != NULL)
                {
                  idx = (idx + 1) % table->cap;
                  newalloc = allocs + idx;
                }
              *newalloc = *alloc;
            }
        }
      free (table->allocs);
      table->allocs = allocs;
    }
  idx = xmlHashAddress ((xmlUIntPtr) p) % table->cap;
  alloc = table->allocs + idx;
  while (alloc->p != NULL)
    {
      idx = (idx + 1) % table->cap;
      alloc = table->allocs + idx;
    }
  alloc->isTombstone = LXML_FALSE;
  alloc->size = size;
  alloc->p = p;
  table->cnt++;
}

static xmlAlloc *
xmlMoveAlloc (xmlAllocTable *table, void *p, void *np)
{
  xmlAlloc *alloc;
  xmlUIntPtr idx;
  assert (table != NULL);
  assert (p != NULL);
  assert (np != NULL);
  idx = xmlHashAddress ((xmlUIntPtr) p) % table->cap;
  alloc = table->allocs + idx;
  while (alloc->p != NULL || alloc->isTombstone == LXML_TRUE)
    {
      if (p == alloc->p)
        {
          xmlAlloc *newalloc;
          assert (alloc->isTombstone == LXML_FALSE);
          idx = xmlHashAddress ((xmlUIntPtr) np) % table->cap;
          newalloc = table->allocs + idx;
          while (newalloc->p != NULL)
            {
              if (alloc == newalloc)
                {
                  alloc->p = np;
                  return alloc;
                }
              idx = (idx + 1) % table->cap;
              newalloc = table->allocs + idx;
            }
          *newalloc = *alloc;
          newalloc->p = np;
          alloc->p = NULL;
          alloc->isTombstone = LXML_TRUE;
          return newalloc;
        }
      idx = (idx + 1) % table->cap;
      alloc = table->allocs + idx;
    }
  assert (0);
  return NULL;
}

static void *
xmlDebugMalloc (xmlSize size, void *ctx)
{
  xmlDebugAllocator *debugAllocator;
  void *p;
  assert (ctx != NULL);
  debugAllocator = (xmlDebugAllocator *) ctx;
  p = debugAllocator->allocator->malloc (size, debugAllocator->allocator->ctx);
  assert (p != NULL);
  xmlAppendAlloc (&debugAllocator->table, p, size);
  debugAllocator->bytesAllocated += size;
  return p;
}

static void *
xmlDebugRealloc (void *p, xmlSize size, void *ctx)
{
  xmlDebugAllocator *debugAllocator;
  xmlAlloc *alloc;
  void *np;
  if (p == NULL)
    return xmlDebugMalloc (size, ctx);
  assert (ctx != NULL);
  debugAllocator = (xmlDebugAllocator *) ctx;
  np = debugAllocator->allocator->realloc (p, size, ctx);
  assert (np != NULL);
  alloc = (p == np) ? xmlFindAlloc (&debugAllocator->table, p)
                    : xmlMoveAlloc (&debugAllocator->table, p, np);
  debugAllocator->bytesAllocated += size;
  debugAllocator->bytesFreed += alloc->size;
  alloc->size = size;
  return np;
}

static void
xmlDebugFree (void *p, void *ctx)
{
  xmlDebugAllocator *debugAllocator;
  xmlAlloc *alloc;
  assert (p != NULL);
  assert (ctx != NULL);
  debugAllocator = (xmlDebugAllocator *) ctx;
  alloc = xmlFindAlloc (&debugAllocator->table, p);
  alloc->isTombstone = LXML_TRUE;
  alloc->p = NULL;
  debugAllocator->bytesFreed += alloc->size;
  debugAllocator->table.cnt--;
  debugAllocator->allocator->free (p, debugAllocator->allocator->ctx);
}

xmlError
xmlInitStdAllocator (xmlAllocator **out)
{
  xmlAllocator *stdAllocator;
  if (out == NULL)
    return LXML_ERR_ARG;
  stdAllocator = malloc (sizeof (xmlAllocator));
  if (stdAllocator == NULL)
    return LXML_ERR_ALLOC;
  memset (stdAllocator, 0, sizeof (xmlAllocator));
  stdAllocator->malloc = xmlMalloc;
  stdAllocator->realloc = xmlRealloc;
  stdAllocator->free = xmlFree;
  *out = stdAllocator;
  return LXML_ERR_NONE;
}

xmlError
xmlInitStdDebugAllocator (xmlAllocator **out)
{
  xmlDebugAllocator *stdDebugAllocator;
  xmlError error;
  if (out == NULL)
    return LXML_ERR_ARG;
  stdDebugAllocator = malloc (sizeof (xmlDebugAllocator));
  if (stdDebugAllocator == NULL)
    return LXML_ERR_ALLOC;
  memset (stdDebugAllocator, 0, sizeof (xmlDebugAllocator));
  if ((error = xmlInitStdAllocator (&stdDebugAllocator->allocator))
      != LXML_ERR_NONE)
    {
      xmlDestroyStdAllocator (stdDebugAllocator->allocator);
      return error;
    }
  stdDebugAllocator->isOwning = LXML_TRUE;
  stdDebugAllocator->debug.malloc = xmlDebugMalloc;
  stdDebugAllocator->debug.realloc = xmlDebugRealloc;
  stdDebugAllocator->debug.free = xmlDebugFree;
  stdDebugAllocator->debug.ctx = stdDebugAllocator;
  *out = &stdDebugAllocator->debug;
  return LXML_ERR_NONE;
}

xmlError
xmlInitDebugAllocator (xmlAllocator *allocator, xmlAllocator **out)
{
  xmlDebugAllocator *stdDebugAllocator;
  if (allocator == NULL || out == NULL)
    return LXML_ERR_ARG;
  stdDebugAllocator = malloc (sizeof (xmlDebugAllocator));
  if (stdDebugAllocator == NULL)
    return LXML_ERR_ALLOC;
  memset (stdDebugAllocator, 0, sizeof (xmlDebugAllocator));
  stdDebugAllocator->isOwning = LXML_FALSE;
  stdDebugAllocator->allocator = allocator;
  stdDebugAllocator->debug.malloc = xmlDebugMalloc;
  stdDebugAllocator->debug.realloc = xmlDebugRealloc;
  stdDebugAllocator->debug.free = xmlDebugFree;
  stdDebugAllocator->debug.ctx = &stdDebugAllocator;
  *out = &stdDebugAllocator->debug;
  return LXML_ERR_NONE;
}

xmlError
xmlDebugAllocatorGetMetrics (xmlAllocator *allocator, xmlDebugMetrics *metrics)
{
  xmlDebugAllocator *debugAllocator;
  if (allocator == NULL || metrics == NULL)
    return LXML_ERR_ARG;
  debugAllocator = (xmlDebugAllocator *) allocator;
  metrics->bytesAllocated = debugAllocator->bytesAllocated;
  metrics->bytesFreed = debugAllocator->bytesFreed;
  return LXML_ERR_NONE;
}

xmlError
xmlDestroyStdAllocator (xmlAllocator *allocator)
{
  if (allocator == NULL)
    return LXML_ERR_ARG;
  free (allocator);
  return LXML_ERR_NONE;
}

xmlError
xmlDestroyDebugAllocator (xmlAllocator *allocator)
{
  xmlDebugAllocator *debugAllocator;
  if (allocator == NULL)
    return LXML_ERR_ARG;
  debugAllocator = (xmlDebugAllocator *) allocator;
  if (debugAllocator->isOwning)
    XML_RETURN_ERROR (xmlDestroyStdAllocator (debugAllocator->allocator));
  if (debugAllocator->table.allocs != NULL)
    free (debugAllocator->table.allocs);
  free (debugAllocator);
  return LXML_ERR_NONE;
}
