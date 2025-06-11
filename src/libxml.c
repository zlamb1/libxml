#include "_libxml.h"

static inline xmlError
xmlReadCharacter(xmlParser *parser)
{
    xmlScalarDecode decode;
    if ( parser->pos >= parser->size )
        return LXML_ERR_LEN;
    decode = parser->srcConverter->decode(parser->src + parser->pos, parser->size - parser->pos);
    switch (decode.error) {
        case LXML_ERR_NONE:
            break;
        case LXML_ERR_LEN:
            parser->character = 0;
            return LXML_ERR_LEN;
        default:
            return decode.error;
    }
    if ( decode.error != LXML_ERR_NONE )
        return decode.error;
    parser->character = decode.scalar;
    parser->advance = decode.size;
    return LXML_ERR_NONE;
}

static inline void
xmlConsumeCharacter(xmlParser *parser)
{
    parser->pos += parser->advance;
    parser->advance = 0;
}

static xmlError
xmlParseTagType(xmlParser *parser)
{
    XML_RETURN_ERROR(xmlReadCharacter(parser));
    switch ( parser->character ) {
        default:
            if ( xmlIsNameStartCharacter(parser->character) == LXML_TRUE ) {

            }
            return LXML_ERR_PARSE;
    }
    return LXML_ERR_NONE;
}

static xmlError
xmlProcessCommands(xmlParser *parser)
{
    xmlParseCommand command;
    (void) xmlParseTagType;
    while ( xmlDequeueParseCommand(parser, &command) == LXML_ERR_NONE ) {
        switch ( command.type ) {
            case LXML_PARSER_STATE_TYPE_CHARACTER:
                XML_RETURN_ERROR(xmlReadCharacter(parser));
                if ( parser->character != command.character ) {
                    printf("expected '%c'; got '%c'\n", (char) command.character, (char) parser->character);
                    return LXML_ERR_PARSE;
                }
                break;
            default:
                break;
        }
    }
    return LXML_ERR_NONE;
}

xmlError
xmlParseDocument(xmlChar *src, xmlSize size, xmlParser *parser, xmlDocument **out)
{
    xmlDocument *document;
    xmlError error = LXML_ERR_NONE;
    (void) xmlReadCharacter;
    (void) xmlConsumeCharacter;
    if ( src == NULL || size == 0 || parser == NULL || out == NULL )
        return LXML_ERR_ARG;
    document = parser->allocator->malloc(sizeof(xmlDocument), parser->allocator->ctx);
    if ( document == NULL )
        return LXML_ERR_PARSE;
    memset(document, 0, sizeof(xmlDocument));
    document->version = LXML_VERSION_1_0;
    document->allocator = parser->allocator;
    parser->pos = 0;
    parser->size = size;
    parser->src = (xmlUChar*) src;
    parser->head = 0;
    parser->tail = 0;
    xmlExpectCharacter(parser, LXML_UC_LESS_THAN);
    XML_CHECK_ERROR(xmlProcessCommands(parser), error, fail);
    *out = document;
    return LXML_ERR_NONE;
    fail:
    return error;
}

xmlError
xmlGetDocumentStringIterator(xmlDocument *document, xmlString *string, xmlStringIterator *iterator)
{
    if ( document == NULL || iterator == NULL )
        return LXML_ERR_ARG;
    *iterator = (xmlStringIterator) {
        .pos = 0,
        .string = string
    };
    return LXML_ERR_NONE;
}

xmlError
xmlDestroyDocument(xmlDocument *document)
{
    xmlAllocator *allocator;
    if ( document == NULL )
        return LXML_ERR_ARG;
    allocator = document->allocator;
    allocator->free(document, allocator->ctx);
    return LXML_ERR_NONE;
}
