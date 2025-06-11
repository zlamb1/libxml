#include <assert.h>

#include "_libxml.h"

void
xmlQueueParseCommand(xmlParser *parser, xmlParseCommand command)
{
    assert(parser->head != ((parser->tail + 1) % LXML_PARSER_COMMAND_QUEUE_SIZE));
    parser->queue[parser->tail] = command;
    parser->tail = (parser->tail + 1) % LXML_PARSER_COMMAND_QUEUE_SIZE; 
}

void
xmlExpectCharacter(xmlParser *parser, xmlUTF32 expected)
{
    xmlQueueParseCommand(parser, (xmlParseCommand) {
        .type = LXML_PARSER_STATE_TYPE_CHARACTER,
        .character = expected
    });
}

xmlError
xmlDequeueParseCommand(xmlParser *parser, xmlParseCommand *command)
{
    if ( parser->head == parser->tail )
        return LXML_ERR_LEN;
    *command = parser->queue[parser->head];
    parser->head = (parser->head + 1) % LXML_PARSER_COMMAND_QUEUE_SIZE;
    return LXML_ERR_NONE;
}
