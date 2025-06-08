#include <assert.h>

#include "xml_unicode.h"

#include "_xml_parser.h"

XMLBoolean
XMLIsNameStartChar(unsigned cp)
{
    switch ( cp ) {
        case XML_UNICODE_COLON: // :
        case XML_UNICODE_UNDERSCORE:
            return XML_TRUE;
        default:
            return ( cp >= 0x00041 && cp <= 0x0005A ) || // [A-Z] 
                   ( cp >= 0x00061 && cp <= 0x0007A ) || // [a-z]
                   ( cp >= 0x000C0 && cp <= 0x000D6 ) || 
                   ( cp >= 0x000D8 && cp <= 0x000F6 ) ||
                   ( cp >= 0x000F8 && cp <= 0x002FF ) ||
                   ( cp >= 0x00370 && cp <= 0x0037D ) ||
                   ( cp >= 0x0037F && cp <= 0x01FFF ) ||
                   ( cp >= 0x0200C && cp <= 0x0200D ) ||
                   ( cp >= 0x02070 && cp <= 0x0218F ) ||
                   ( cp >= 0x02C00 && cp <= 0x02FEF ) ||
                   ( cp >= 0x03001 && cp <= 0x0D7FF ) ||
                   ( cp >= 0x0F900 && cp <= 0x0FDCF ) ||
                   ( cp >= 0x0FDF0 && cp <= 0x0FFFD ) ||
                   ( cp >= 0x10000 && cp <= 0xEFFFF );
    }
}

XMLBoolean
XMLIsNameChar(unsigned cp) {
    if ( XMLIsNameStartChar(cp) )
        return XML_TRUE;
    switch ( cp ) {
        case XML_UNICODE_HYPHEN:
        case XML_UNICODE_PERIOD:
        case XML_UNICODE_MIDDLE_POINT:
            return XML_TRUE;
        default:
            return ( cp >= 0x0030 && cp <= 0x0039 ) || // [0-9]
                   ( cp >= 0x0300 && cp <= 0x036F ) ||
                   ( cp >= 0x203F && cp <= 0x2040 );
    } 
}

static int
XMLParseName(XMLParser *parser, XMLString *string)
{
    int advance;
    XMLPeekCP(parser, parser->cp, advance);
    if ( XMLIsNameStartChar(parser->cp) != XML_TRUE )
        return XML_ERR_PARSE;
    while ( XMLIsNameChar(parser->cp) ) {
        XMLStringAppendChecked(string, parser->cp);
        XMLAdvanceCP(parser, advance);
        if ( parser->bytes.atEnd ) 
            break;
        XMLPeekCP(parser, parser->cp, advance);
    }
    return XML_SUCCESS;
}

int 
XMLConsumeWhiteSpace(XMLParser *parser)
{
    int cnt = 0;
    unsigned advance;
    if ( parser->bytes.atEnd )
        return XML_SUCCESS;
    while ( 1 ) {
        XMLPeekCP(parser, parser->cp, advance);
        switch ( parser->cp ) {
            case XML_UNICODE_TAB:
            case XML_UNICODE_CR:
            case XML_UNICODE_NEWLINE:
            case XML_UNICODE_SPACE:
                cnt++;
                XMLAdvanceCP(parser, advance);
                break;
            default:
                return cnt;
        }
    }
}

static int
XMLParseEq(XMLParser *parser)
{
    int result;
    XMLConsumeWhiteSpaceChecked(parser, result);
    XMLExpectCP(parser, XML_UNICODE_EQUALS, result);
    if ( result == 0 ) {
        char msg[8];
        XMLCharToString(msg, parser->codec, parser->cp);
        XMLAllocateErrorMessageChecked(parser, "expected '=' for attribute value: got %s\n", msg);
        return XML_ERR_PARSE;
    }
    XMLConsumeWhiteSpaceChecked(parser, result); 
    return XML_SUCCESS;
}

static int
XMLParseAttribValue(XMLParser *parser, XMLString *value)
{
    unsigned cp;
    int advance;
    XMLPeekCP(parser, parser->cp, advance);
    switch ( parser->cp ) {
        case XML_UNICODE_QUOTE:
        case XML_UNICODE_APOSTROPHE:
            cp = parser->cp;
            break;
        default: {
            char msg[8]; 
            XMLCharToString(msg, parser->codec, parser->cp);
            XMLAllocateErrorMessageChecked(parser, "expected '\"' for attribute value: got %s\n", msg);
            return XML_ERR_PARSE;
        }
    }
    XMLAdvanceCP(parser, advance);
    while (1) {
        XMLPeekCP(parser, parser->cp, advance);
        if ( parser->cp == cp ) {
            XMLAdvanceCP(parser, advance);
            break;
        }
        switch ( parser->cp ) {
            case XML_UNICODE_NUL:
                XMLSetErrorMessage(parser, "unexpected EOF\n"); 
                return XML_ERR_PARSE;
            case XML_UNICODE_AMPERSAND:
            case XML_UNICODE_LESS_THAN: {
                char msg[8]; 
                XMLCharToString(msg, parser->codec, parser->cp);
                XMLAllocateErrorMessageChecked(parser, "invalid character within attribute value: %s\n", msg);
                return XML_ERR_PARSE;
            }
        }
        XMLStringAppendChecked(value, parser->cp); 
        XMLAdvanceCP(parser, advance);
    }
    return XML_SUCCESS;
}

int
XMLParseStartTag(XMLParser *parser)
{
    XMLNode *node;
    XMLResult result;
    char msg[8]; 
    if ( parser->root == NULL ) {
        XMLAllocateChecked(node, parser->allocator, sizeof(XMLNode));
        node->parent = NULL;
        parser->root = node;
    } else {
        XMLNode tmp;
        if ( parser->root->isClosed == XML_TRUE ) {
            XMLSetErrorMessage(parser, "unexpected tag after root node\n");
            return XML_ERR_PARSE;
        }
        tmp = (XMLNode) {0};
        assert(parser->node != NULL);
        assert(parser->node->nodeType == XML_NODE_TYPE_EMPTY || parser->node->nodeType == XML_NODE_TYPE_MIXED);
        if ( parser->node->nodeType != XML_NODE_TYPE_MIXED ) {
            parser->node->nodeType = XML_NODE_TYPE_MIXED;
            parser->node->children.cap = 0;
            parser->node->children.nchildren = 0;
            parser->node->children.nodes = NULL;
        }
        XMLCheckError(XMLNodeAppendChild(parser->node, tmp, parser->allocator));
        assert(parser->node->children.nchildren > 0);
        assert(parser->node->isClosed == XML_FALSE);
        node = parser->node->children.nodes + ( parser->node->children.nchildren - 1 );
        node->parent = parser->node;
    }
    node->name = InitXMLString(parser->codec, parser->allocator); 
    node->isClosed = XML_FALSE;
    node->nattribs = 0;
    node->cattribs = 0;
    node->attribs = NULL;
    node->nodeType = XML_NODE_TYPE_EMPTY;
    parser->node = node;
    result = XMLParseName(parser, &node->name);
    switch ( result ) {
        case XML_SUCCESS:
            break;
        case XML_ERR_PARSE:
            XMLCharToString(msg, parser->codec, parser->cp);
            XMLAllocateErrorMessageChecked(parser, "expected tag name: got %s\n", msg);
            return XML_ERR_PARSE;
        default:
            return result;
    }
    XMLConsumeWhiteSpaceChecked(parser, result);
    while ( 1 ) {
        XMLNodeAttribute attrib = { 
            .name  = InitXMLString(parser->codec, parser->allocator),
            .value = InitXMLString(parser->codec, parser->allocator)
        };
        result = XMLParseName(parser, &attrib.name);
        switch ( result ) {
            case XML_SUCCESS:
                break;
            case XML_ERR_PARSE:
                goto afterAttribs;
            default:
                return result;
        }
        XMLCheckError(XMLParseEq(parser));
        XMLCheckError(XMLParseAttribValue(parser, &attrib.value)); 
        XMLCheckError(XMLNodeAppendAttrib(node, attrib, parser->allocator)); 
        XMLConsumeWhiteSpaceChecked(parser, result);
        if ( result == 0 ) 
            break;
    }
    afterAttribs:
    XMLPeekCP(parser, parser->cp, result);
    switch ( parser->cp ) {
        case XML_UNICODE_SLASH:
            XMLAdvanceCP(parser, result);
            XMLExpectCP(parser, XML_UNICODE_GREATER_THAN, result);
            if ( result == 0 ) 
                goto NoEndTag;
            node->isClosed = XML_TRUE;
            parser->node = node->parent;
            break;
        case XML_UNICODE_GREATER_THAN:
            XMLAdvanceCP(parser, result);
            break;
        default:
            NoEndTag:
            XMLCharToString(msg, parser->codec, parser->cp);
            XMLAllocateErrorMessageChecked(parser, "expected end of tag specifier '>': got %s\n", msg);
            return XML_ERR_PARSE;
    }
    return XML_SUCCESS;
}

int
XMLParseEndTag(XMLParser *parser, XMLString *endTag)
{
    char msg[8]; 
    int result;
    assert(endTag != NULL);
    if ( ( result = XMLParseName(parser, endTag) ) != XML_SUCCESS ) {
        switch (result) {
            case XML_ERR_PARSE:
                XMLSetErrorMessage(parser, "expected end tag name\n"); 
                return XML_ERR_PARSE;
            default:
                return result;
        }
    }
    XMLConsumeWhiteSpaceChecked(parser, result);
    XMLExpectCP(parser, parser->cp, result);
    if ( result == 0 ) {
        XMLCharToString(msg, parser->codec, parser->cp);
        XMLAllocateErrorMessageChecked(parser, "expected end of tag specifier '>': got %s\n", msg);
        return XML_ERR_PARSE;
    }
    return XML_SUCCESS;
}
