#ifndef XMLENTITY_H
#define XMLENTITY_H

#include "xmlstring.h"

typedef struct xmlEntity
{
  xmlString name;
  xmlString text;
} xmlEntity;

#endif
