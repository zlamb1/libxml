#ifndef XMLALLOC_H
#define XMLALLOC_H 1

#include "xmltype.h"

typedef struct
xmlAllocator
{
    void *(*malloc)(xmlSize size, void *ctx);
    void *(*realloc)(void *p, xmlSize size, void *ctx);
    void  (*free)(void *p, void *ctx);
    void *ctx;
} xmlAllocator;

typedef struct
xmlDebugMetrics
{
    xmlSize bytesAllocated;
    xmlSize bytesFreed;
} xmlDebugMetrics;

xmlError
xmlInitStdAllocator(xmlAllocator **out); 

xmlError
xmlInitStdDebugAllocator(xmlAllocator **out);

xmlError
xmlInitDebugAllocator(xmlAllocator *allocator, xmlAllocator **out);  

xmlError
xmlDebugAllocatorGetMetrics(xmlAllocator *allocator, xmlDebugMetrics *metrics);

xmlError
xmlDestroyStdAllocator(xmlAllocator *allocator);

xmlError
xmlDestroyDebugAllocator(xmlAllocator *allocator);

#endif
