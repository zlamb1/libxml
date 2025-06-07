#include <assert.h>

#include "xml_parse.h"
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
_XMLParseName(_XMLParser *parser, XMLString *string)
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
    (void) string;
    return XML_SUCCESS;
    XMLAllocateFail:
    return XML_ERR_ALLOC;
    XMLPeekFail:
    return advance;
}

#define XMLConsumeWhiteSpaceChecked(PARSER, RESULT) \
    do { \
        if ( ( (RESULT) = _XMLConsumeWhiteSpace(parser) ) < 0 ) \
            return (RESULT); \
    } while (0)

static int 
_XMLConsumeWhiteSpace(_XMLParser *parser)
{
    int cnt = 0;
    unsigned advance;
    assert(parser != NULL);
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
    XMLPeekFail:
    return advance;
}

static int
_XMLParseEq(_XMLParser *parser)
{
    int result;
    if ( ( result = _XMLConsumeWhiteSpace(parser) ) < 0 )
        return result;
    XMLExpectCP(parser, XML_UNICODE_EQUALS, result);
    if ( result == 0 ) {
        char msg[8];
        XMLCharToString(msg, parser->codec, parser->cp);
        XMLAllocateErrorMessageChecked(parser, "expected '=' for attribute value: got %s\n", msg);
        return XML_ERR_PARSE;
    }
    if ( ( result = _XMLConsumeWhiteSpace(parser) ) < 0 )
        return result;
    return XML_SUCCESS;
    XMLAllocateFail:
    return XML_ERR_ALLOC;
    XMLExpectFail:
    return result;
}

static int
_XMLParseAttribValue(_XMLParser *parser, XMLString *value)
{
    unsigned cp;
    int advance;
    XMLPeekCP(parser, parser->cp, advance);
    switch ( parser->cp ) {
        case XML_UNICODE_QUOTE:
            // fallthrough
        case XML_UNICODE_APOSTROPHE:
            cp = parser->cp;
            break;
        default: {
            char msg[8]; 
            XMLCharToString(msg, parser->codec, parser->cp);
            XMLAllocateErrorMessageChecked(parser, "expected quote '\"' for attribute value: got %s\n", msg);
            return XML_ERR_PARSE;
        }
    }
    XMLAdvanceCP(parser, advance);
    while ( 1 ) {
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
                XMLAllocateErrorMessageChecked(parser, "invalid character %s within attribute value\n", msg);
                return XML_ERR_PARSE;
            }
        }
        XMLStringAppendChecked(value, parser->cp); 
        XMLAdvanceCP(parser, advance);
    }
    return XML_SUCCESS;
    XMLAllocateFail:
    return XML_ERR_ALLOC;
    XMLPeekFail:
    return advance;
}

static int
_XMLParseCharacterData(_XMLParser *parser, XMLNode *parent)
{
    (void) parser;
    (void) parent;
    return 0;
}

int
XMLParseStartTag(XMLParser *base)
{
    _XMLParser *parser;
    XMLNode *node;
    char msg[8]; 
    int result;
    assert(base != NULL);
    parser = (_XMLParser *) base;
    if ( parser->root == NULL ) {
        XMLAllocateChecked(node, parser->base.allocator, sizeof(XMLNode));
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
        if ( XMLNodeAppendChild(parser->node, tmp, parser->base.allocator) != XML_SUCCESS )
            goto XMLAllocateFail;
        assert(parser->node->children.nchildren > 0);
        assert(parser->node->isClosed == XML_FALSE);
        node = parser->node->children.nodes + ( parser->node->children.nchildren - 1 );
        node->parent = parser->node;
        parser->node->nodeType = XML_NODE_TYPE_MIXED;
    }
    node->name = InitXMLString(parser->codec, parser->base.allocator); 
    node->isClosed = XML_FALSE;
    node->nattribs = 0;
    node->cattribs = 0;
    node->attribs = NULL;
    node->nodeType = XML_NODE_TYPE_EMPTY;
    parser->node = node;
    result = _XMLParseName(parser, &node->name);
    switch (result) {
        case XML_ERR_PARSE:
            XMLCharToString(msg, parser->codec, parser->cp);
            XMLAllocateErrorMessageChecked(parser, "expected tag name: got %s\n", msg);
            return XML_ERR_PARSE;
        case XML_SUCCESS:
            break;
        default:
            return result;
    }
    XMLConsumeWhiteSpaceChecked(parser, result);
    while ( 1 ) {
        XMLNodeAttribute attrib = { 
            .name  = InitXMLString(parser->codec, parser->base.allocator),
            .value = InitXMLString(parser->codec, parser->base.allocator)
        };
        if ( ( result = _XMLParseName(parser, &attrib.name) ) == XML_ERR_PARSE )
            break;
        if ( ( result = _XMLParseEq(parser) ) != XML_SUCCESS )
            return result;
        if ( ( result = _XMLParseAttribValue(parser, &attrib.value) ) != XML_SUCCESS )
            return result;
        if ( ( result = XMLNodeAppendAttrib(node, attrib, parser->base.allocator) ) != XML_SUCCESS )
            return result; 
        if ( ( result = _XMLConsumeWhiteSpace(parser) ) < 0)
            return result;
        else if ( result == 0 ) {
            break;
        }
    }
    XMLPeekCP(parser, parser->cp, result);
    switch ( parser->cp ) {
        case XML_UNICODE_SLASH:
            XMLAdvanceCP(parser, result);
            XMLExpectCP(parser, XML_UNICODE_GREATER_THAN, result);
            if ( result == 0 ) 
                goto NoEndTag;
            node->isClosed = XML_TRUE;
            parser->node = node->parent;
            // fallthrough
        case XML_UNICODE_GREATER_THAN:
            break;
        default:
            NoEndTag:
            XMLCharToString(msg, parser->codec, parser->cp);
            XMLAllocateErrorMessageChecked(parser, "expected end tag '>': got %s\n", msg);
            return XML_ERR_PARSE;
    }
    
    (void) msg; 
    (void) _XMLParseName;
    (void) _XMLParseCharacterData;
    (void) _XMLParseEq;
    return XML_SUCCESS;
    XMLAllocateFail:
    XMLSetErrorMessage(parser, "failed to allocate memory\n");
    return XML_ERR_ALLOC;
    XMLPeekFail:
    XMLExpectFail:
    return XML_ERR_ENCODING;
}
