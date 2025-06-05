#include <stdio.h>

#include "xmlparser.h"

char *
readfile(const char *name)
{
    FILE *fp;
    char *buf;
    long size, read;
    if ( fopen_s(&fp, name, "rb") != 0 )
        return NULL;
    if ( fseek(fp, 0L, SEEK_END) != 0 || ( size = ftell(fp) ) == -1L || fseek(fp, 0L, SEEK_SET) != 0 )
        goto fail;
    buf = malloc(size + 1);
    if ( buf == NULL )
        goto fail;
    buf[size] = '\0';
    if ( ( read = fread(buf, 1, size, fp) ) != size ) {
        free(buf);
        goto fail;
    }
    fclose(fp);
    return buf;
    fail:
    fclose(fp);
    return NULL;
}

static const char *EXAMPLE_FILES[1] = {
    "./examples/example01.xml"
};

int 
main(void)
{
    char *buf;
    XMLParser *parser;
    if ( InitXMLParser(NULL, &parser) != XML_SUCCESS ) {
        printf("failed to init XMLParser\n");
        return -1;
    } 
    for (unsigned i = 0; i < sizeof(EXAMPLE_FILES) / sizeof(const char *); i++) {
        XMLDocument document;
        int result;
        printf("reading %s...\n", EXAMPLE_FILES[i]);
        buf = readfile(EXAMPLE_FILES[i]);
        if ( buf == NULL ) {
            printf("failed to read %s\n", EXAMPLE_FILES[i]);
            continue;
        }
        if ( ( result = ParseXML(parser, buf, &document) ) == XML_SUCCESS ) {
            printf("parsed XMLDocument\n");
        } else {
            printf("failed to parse XMLDocument -> %i\n", result);
        }
        free(buf);
    }
    DestroyXMLParser(parser);
    return 0;
}