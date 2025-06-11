#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "libxml.h"

static char *
readfile(const char *name, size_t *sz)
{
    FILE *fp;
    char *buf;
    long size, read;
    if ( fopen_s(&fp, name, "rb") != 0 )
        return NULL;
    if ( fseek(fp, 0L, SEEK_END) != 0 || ( size = ftell(fp) ) == -1L || fseek(fp, 0L, SEEK_SET) != 0 )
        goto fail;
    buf = malloc(size + 4);
    if ( buf == NULL )
        goto fail;
    memset(buf + size, 0, 4);
    if ( ( read = fread(buf, 1, size, fp) ) != size ) {
        free(buf);
        goto fail;
    }
    if ( sz != NULL )
        *sz = size; 
    fclose(fp);
    return buf;
    fail:
    fclose(fp);
    return NULL;
}

static const char *EXAMPLE_FILES[1] = {
    "./tests/example01.xml"
};

int 
main(void)
{
    xmlParser *parser;
    char *buf;
    size_t len; 
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    if ( xmlInitParser(NULL, &parser) != LXML_ERR_NONE ) {
        printf("failed to init parser\n");
        return -1;
    }
    for (size_t i = 0; i < sizeof(EXAMPLE_FILES) / sizeof(const char *); i++) {
        xmlDocument *document;
        xmlError error = LXML_ERR_NONE;
        printf("reading %s...\n", EXAMPLE_FILES[i]);
        buf = readfile(EXAMPLE_FILES[i], &len);
        if ( buf == NULL ) {
            printf("failed to read: %s\n", EXAMPLE_FILES[i]);
            continue;
        }
        error = xmlParseDocument(buf, len, parser, &document);
        if ( error == LXML_ERR_NONE ) {
            printf("parsed document\n");
        } else {
            printf("failed to parse document -> %i\n", error);
        }
        free(buf);
    }
    if ( xmlDestroyParser(parser) != LXML_ERR_NONE ) {
        printf("failed to destroy parser\n");
        return -1;
    }
    return 0;
}