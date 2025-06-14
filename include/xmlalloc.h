#ifndef XMLALLOC_H
#define XMLALLOC_H 1

#include "xmltype.h"

#define XML_MAYBE_EXPAND(BUF, TYPE, SIZE, CAP, ALLOCATOR)                     \
  do                                                                          \
    {                                                                         \
      xmlSize cap = (CAP);                                                    \
      if (cap == 0)                                                           \
        (CAP) = 1;                                                            \
      while ((SIZE) >= (CAP))                                                 \
        (CAP) *= 2;                                                           \
      if (cap != (CAP))                                                       \
        {                                                                     \
          TYPE *buf = (ALLOCATOR)->realloc ((BUF), sizeof (TYPE) * (CAP),     \
                                            (ALLOCATOR)->ctx);                \
          if (buf == NULL)                                                    \
            return LXML_ERR_ALLOC;                                            \
          (BUF) = buf;                                                        \
        }                                                                     \
    }                                                                         \
  while (0)

typedef struct xmlAllocator
{
  void *(*malloc) (xmlSize size, void *ctx);
  void *(*realloc) (void *p, xmlSize size, void *ctx);
  void (*free) (void *p, void *ctx);
  void *ctx;
} xmlAllocator;

typedef struct xmlDebugMetrics
{
  xmlSize bytesAllocated;
  xmlSize bytesFreed;
} xmlDebugMetrics;

xmlError xmlInitStdAllocator (xmlAllocator **out);

xmlError xmlInitStdDebugAllocator (xmlAllocator **out);

xmlError xmlInitDebugAllocator (xmlAllocator *allocator, xmlAllocator **out);

xmlError xmlDebugAllocatorGetMetrics (xmlAllocator *allocator,
                                      xmlDebugMetrics *metrics);

xmlError xmlDestroyStdAllocator (xmlAllocator *allocator);

xmlError xmlDestroyDebugAllocator (xmlAllocator *allocator);

#endif
