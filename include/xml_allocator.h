#ifndef XML_ALLOCATOR_H
#define XML_ALLOCATOR_H 1

#define XMLAllocate(ALLOCATOR, SIZE) (ALLOCATOR)->malloc((SIZE), (ALLOCATOR)->ctx)

#define XMLAllocateChecked(VAR, ALLOCATOR, SIZE) \
    do { \
        (VAR) = XMLAllocate((ALLOCATOR), (SIZE)); \
        if ( (VAR) == NULL ) \
            goto XMLAllocateFail; \
    } while (0)

typedef struct
XMLAllocator
{
    void *(*malloc)(size_t size, void *ctx);
    void *(*realloc)(void *p, size_t size, void *ctx);
    void  (*free)(void *p, void *ctx); 
    void *ctx;
} XMLAllocator;

#endif
