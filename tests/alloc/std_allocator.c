#include <stdio.h>

#include "xmlalloc.h"

int
main(void)
{
    xmlAllocator *allocator;
    if ( xmlInitStdAllocator(&allocator) != LXML_ERR_NONE )
        return -1;
    for ( unsigned i = 0; i < 100; i++ ) {
        void *p = allocator->malloc(100, allocator->ctx);
        p = allocator->realloc(p, 200, allocator->ctx);
        allocator->free(p, allocator->ctx);
    }
    xmlDestroyStdAllocator(allocator);
    printf("allocated\n");
    return 0;
}
