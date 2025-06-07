#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "xml_parser.h"
#include "xml_unicode.h"

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
    buf = malloc(size + 1);
    if ( buf == NULL )
        goto fail;
    buf[size] = '\0';
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

static void
printstring(XMLString *string)
{
    if ( string->len > 0 )
        printf("%s", string->buf); 
}

static void
printattributes(XMLNode *node)
{
    for ( size_t i = 0; i < node->nattribs; i++ ) {
        printstring(&node->attribs[i].name);
        printf("=\"");
        printstring(&node->attribs[i].value); 
        printf("\"");
        if ( i != node->nattribs -1 )
            printf(" ");
    }
}

static void printstarttag(XMLNode *node)
{
    printf("<%s", node->name.buf);
    if ( node->nattribs ) {
        printf(" ");
        printattributes(node);
        printf(" ");
    }
    if ( node->nodeType == XML_NODE_TYPE_EMPTY )
        printf("/");
    printf(">\n");
}

static void printnode(XMLNode *node)
{
    switch ( node->nodeType ) {
        case XML_NODE_TYPE_EMPTY:
            printstarttag(node);
            break;
        case XML_NODE_TYPE_TEXT:
            printstring(&node->text);
            break;
        case XML_NODE_TYPE_MIXED:
            printstarttag(node);
            for ( size_t i = 0; i < node->children.nchildren; i++ )
                printnode(node->children.nodes + i);
            printf("</%s>", node->name.buf);
            break;
    }
}

static void 
printdocument(XMLDocument *document)
{
    XMLNode *root = document->root;
    printnode(root);
}

static const char *EXAMPLE_FILES[1] = {
    "./examples/example01.xml"
};

int 
main(void)
{
    char *buf;
    size_t len; 
    XMLParser *parser;
    if ( InitXMLParser(NULL, &parser) != XML_SUCCESS ) {
        printf("failed to init XMLParser\n");
        return -1;
    } 
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    for (size_t i = 0; i < sizeof(EXAMPLE_FILES) / sizeof(const char *); i++) {
        XMLDocument document;
        int result;
        printf("reading %s...\n", EXAMPLE_FILES[i]);
        buf = readfile(EXAMPLE_FILES[i], &len);
        if ( buf == NULL ) {
            printf("failed to read %s\n", EXAMPLE_FILES[i]);
            continue;
        }
        if ( ( result = ParseXML(parser, buf, len, &document) ) == XML_SUCCESS ) {
            printdocument(&document);
        } else {
            printf("failed to parse XMLDocument -> %i\n", result);
            printf("%s\n", GetXMLParserError(parser));
        }
        free(buf);
    }
    DestroyXMLParser(parser);
    return 0;
}