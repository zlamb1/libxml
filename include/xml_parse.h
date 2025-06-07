#ifndef XML_PARSE_H
#define XML_PARSE_H 1

#include "xml_parser.h"

XMLBoolean
XMLIsNameStartChar(unsigned cp);

XMLBoolean
XMLIsNameChar(unsigned cp);

int
XMLParseStartTag(XMLParser *base);

#endif
