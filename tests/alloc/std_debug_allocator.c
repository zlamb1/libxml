#include <stdio.h>

#include "xmlalloc.h"

int
main (void)
{
  xmlAllocator *allocator;
  xmlDebugMetrics metrics;
  if (xmlInitStdDebugAllocator (&allocator) != LXML_ERR_NONE)
    return -1;
  for (unsigned i = 0; i < 100; i++)
    {
      void *p = allocator->malloc (100, allocator->ctx);
      p = allocator->realloc (p, 200, allocator->ctx);
      allocator->free (p, allocator->ctx);
    }
  if (xmlDebugAllocatorGetMetrics (allocator, &metrics) != LXML_ERR_NONE)
    return -2;
  printf ("Bytes Allocated: %zu\n", metrics.bytesAllocated);
  printf ("Bytes Freed: %zu\n", metrics.bytesFreed);
  xmlDestroyDebugAllocator (allocator);
  return 0;
}
