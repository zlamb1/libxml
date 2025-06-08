#ifndef XML_TYPE_H
#define XML_TYPE_H 1

typedef enum 
XMLResult
{
    XML_SUCCESS              =  0,
    XML_ERR_ALLOC            = -1,
    XML_ERR_PARSE            = -2,
    XML_ERR_ENCODING         = -3,
    XML_ERR_BUF_SIZE         = -4,
    XML_ERR_SURROGATE        = -5,
    XML_ERR_OVERLONG         = -6,
    XML_ERR_INVALID_ARG      = -7,
    XML_ERR_INVALID_PARSER   = -8,
    XML_ERR_INVALID_DOCUMENT = -9
} XMLResult;

#define XML_FALSE 0
#define XML_TRUE  1

typedef int XMLBoolean;

#endif
