#include <string.h>

#include "xmlnode.h"

xmlError
xmlNodeAppendChild (xmlNode *parent, xmlNode **out, xmlAllocator *allocator)
{
  xmlNode *node, *c;
  switch (parent->type)
    {
    case LXML_NODE_TYPE_EMPTY:
      parent->type = LXML_NODE_TYPE_MIXED;
      parent->children = NULL;
      parent->nchildren = 0;
      parent->cchildren = 0;
    case LXML_NODE_TYPE_MIXED:
      break;
    default:
      return LXML_ERR_ARG;
    }
  c = parent->children;
  XML_MAYBE_EXPAND (parent->children, xmlNode, parent->nchildren,
                    parent->cchildren, allocator);
  if (c != parent->children)
    {
      for (xmlSize i = 0; i < parent->nchildren; i++)
        {
          /* update child pointers to moved parents */
          xmlNode *child = parent->children + i;
          if (child->type == LXML_NODE_TYPE_MIXED)
            {
              for (xmlSize j = 0; j < child->nchildren; j++)
                (child->children + j)->parent = child;
            }
        }
    }
  node = parent->children + parent->nchildren++;
  memset (node, 0, sizeof (xmlNode));
  node->parent = parent;
  *out = node;
  return LXML_ERR_NONE;
}

xmlError
xmlNodeGetTreeMemorySize (xmlNode *root, xmlSize *out)
{
  long top = -1, cap = 64;
  xmlNode **stk;
  xmlSize bytes = 0;
  if (root == NULL || out == NULL)
    return LXML_ERR_ARG;
  stk = malloc (sizeof (xmlNode **) * cap);
  if (stk == NULL)
    return LXML_ERR_ALLOC;
  stk[++top] = root;
  while (top > -1)
    {
      xmlNode *node = stk[top--];
      switch (node->type)
        {
        case LXML_NODE_TYPE_MIXED:
          bytes += node->cchildren * sizeof (xmlNode);
          for (xmlSize i = 0; i < node->nchildren; i++)
            {
              xmlNode *child = node->children + i;
              if (top >= cap - 1)
                {
                  xmlNode **newstk;
                  while (top >= cap - 1)
                    cap *= 2;
                  newstk = realloc (stk, sizeof (xmlNode **) * cap);
                  if (newstk == NULL)
                    {
                      free (stk);
                      return LXML_ERR_ALLOC;
                    }
                  stk = newstk;
                }
              stk[++top] = child;
            }
          // fallthrough
        case LXML_NODE_TYPE_EMPTY:
          bytes += node->name.cap;
          bytes += node->cattribs * sizeof (xmlNodeAttribute);
          for (xmlSize i = 0; i < node->nattribs; i++)
            {
              xmlNodeAttribute *attrib = node->attribs + i;
              bytes += attrib->name.cap;
              bytes += attrib->value.cap;
            }
          break;
        case LXML_NODE_TYPE_TEXT:
          bytes += node->text.cap;
          break;
        case LXML_NODE_TYPE_COMMENT:
          bytes += node->comment.cap;
          break;
        }
    }
  *out = bytes;
  free (stk);
  return LXML_ERR_NONE;
}

xmlError
xmlDestroyNode (xmlNode *root, xmlAllocator *allocator, xmlBoolean destroyRoot)
{
  xmlNode *node = root;
  if (root == NULL || allocator == NULL)
    return LXML_ERR_ARG;
  while (node != NULL)
    {
      switch (node->type)
        {
        case LXML_NODE_TYPE_MIXED:
          if (node->nchildren)
            {
              node->nchildren--;
              node = node->children + node->nchildren;
              break;
            }
          if (node->children != NULL)
            {
              allocator->free (node->children, allocator->ctx);
              node->children = NULL;
            }
          // fallthrough
        case LXML_NODE_TYPE_EMPTY:
          xmlDestroyString (&node->name, allocator);
          if (node->attribs != NULL)
            {
              for (xmlSize i = 0; i < node->nattribs; i++)
                {
                  xmlNodeAttribute *attrib = node->attribs + i;
                  if (attrib->name.buf != NULL)
                    xmlDestroyString (&attrib->name, allocator);
                  if (attrib->value.buf != NULL)
                    xmlDestroyString (&attrib->value, allocator);
                }
              allocator->free (node->attribs, allocator->ctx);
              node->attribs = NULL;
            }
          node = node->parent;
          break;
        case LXML_NODE_TYPE_TEXT:
          if (node->text.buf != NULL)
            xmlDestroyString (&node->text, allocator);
          node = node->parent;
          break;
        case LXML_NODE_TYPE_COMMENT:
          if (node->comment.buf != NULL)
            xmlDestroyString (&node->comment, allocator);
          node = node->parent;
          break;
        }
    }
  if (destroyRoot == LXML_TRUE)
    allocator->free (root, allocator->ctx);
  return LXML_ERR_NONE;
}
