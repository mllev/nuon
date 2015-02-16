#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "exec.h"

static inline int strlensafe (unsigned char *str)
{
  int l = strlen((const char *)str);
  return (l > 512 ? 512 : l);
}

node_data_t* exec_addNode(node_data_t* root, unsigned char* ident) 
{
  int len;
  node_data_t* node;

  len = strlensafe(ident);
  node = malloc(sizeof(node_data_t));

  strncpy((char *)node->ident, (char *)ident, len);
  node->ident[len] = 0;

  node->next = NULL;

  if ( root ) {
    while ( root->next ) { 
      if ( !strncmp((const char *)root->ident, (const char *)ident, len) ) {
        free(node);
        return root;
      }
      root = root->next;
    }
    root->next = node;
  }

  return node;
}

void exec_addLabelToNode(node_data_t* node, unsigned char* label) 
{
  int len;

  len = strlensafe(label);
  strncpy((char *)node->label, (char *)label, len);
  node->label[len] = 0;
}

node_data_t* exec_findNode(node_data_t* root, unsigned char* ident) 
{
  int len;
  node_data_t* iter;

  len = strlensafe(ident);
  iter = root;

  while ( iter ) {
    if ( !strncmp((char *)iter->ident, (char *)ident, len) ) {
      break;
    }
    iter = iter->next;
  }
  return iter;
}

void exec_addProperty(node_data_t* node, unsigned char* key, unsigned char* val) 
{
  int klen, vlen, index;

  klen = strlensafe(key);
  vlen = strlensafe(val);
  index = node->propcount;

  strncpy((char *)node->keys[index], (char *)key, klen);
  strncpy((char *)node->vals[index], (char *)val, vlen);

  node->keys[index][klen] = 0;
  node->vals[index][vlen] = 0;

  (node->propcount)++;
}

edge_data_t* exec_addEdge(edge_data_t* root, unsigned char* ident) 
{
  int len;
  edge_data_t* edge;

  edge = malloc(sizeof(edge_data_t));

  if ( ident ) {
    len = strlensafe(ident);
    strncpy((char *)edge->ident, (char *)ident, len);
    edge->ident[len] = 0;
  }

  edge->node_r = NULL;
  edge->node_l = NULL;
  edge->next = NULL;

  if ( root ) {
    while ( root->next ) { root = root->next; }
    root->next = edge;
  }

  return edge;
}

void exec_setRightNode(node_data_t* node, edge_data_t* edge) 
{
  edge->node_r = node;
}

void exec_setLeftNode(node_data_t* node, edge_data_t* edge) 
{
  edge->node_l = node;
}

void exec_addLabelToEdge(edge_data_t* edge, unsigned char* label) 
{
  int len;

  len = strlensafe(label);
  strncpy((char *)edge->label, (char *)label, len);
  edge->label[len] = 0;
}

void exec_cmd (Graph* g, char* cmd, node_data_t* root, edge_data_t* edges)
{
  Vertex *type, *node;
  VertexContainer* returnData;
  Property* prop_iter;
  node_data_t* node_iter = root;
  edge_data_t* edge_iter = edges;
  int count = 0, id;

  if ( !strncmp(cmd, "match", 5) ) {
    while ( node_iter ) {
      count = node_iter->propcount;
      if ( !count ) {
        returnData = graph_getVertices(g, NULL, NULL, NULL);
        while ( returnData ) {
          printf("{\n");
          prop_iter = returnData->vertex->properties;
          while ( prop_iter ) {
            printf("  %s : %s", prop_iter->key, prop_iter->val);
            if ( prop_iter->next ) {
              printf(",");
            }
            printf("\n");
            prop_iter = prop_iter->next;
          }
          printf("}");
          if ( returnData->next ) {
            printf(",\n\n");
          } else {
            printf("\n\n");
          }
          returnData = returnData->next;
        }
      }
      while (count) {
        count--;
        returnData = graph_getVertices(g, node_iter->label, node_iter->keys[count], node_iter->vals[count]);
        while ( returnData ) {
          printf("{\n");
          prop_iter = returnData->vertex->properties;
          while ( prop_iter ) {
            printf("  %s : %s", prop_iter->key, prop_iter->val);
            if ( prop_iter->next ) {
              printf(",");
            }
            printf("\n");
            prop_iter = prop_iter->next;
          }
          printf("}");
          if ( returnData->next ) {
            printf(",\n\n");
          } else {
            printf("\n\n");
          }
          returnData = returnData->next;
        }
      }
      node_iter = node_iter->next;
    }
    return;
  }

  while ( node_iter ) {
    char key[256];
    type = graph_getVertex(g, node_iter->label);

    if ( !type ) {
      type = graph_setVertex(g, node_iter->label, NULL);
    }

    id = type->idx++;

    sprintf(key, "%s:%d", node_iter->label, id);

    node = graph_setVertex(g, (unsigned char*)key, NULL);
    node_iter->ptr = node;
    graph_vertexAddEdge(type, node, (unsigned char*)"member");
    count = node_iter->propcount;

    while (count) {
      count--;
      graph_vertexSetProperty(node, node_iter->keys[count], node_iter->vals[count]);
    }

    node = NULL;
    type = NULL;

    node_iter = node_iter->next;
  }

  while ( edge_iter ) { 
    graph_vertexAddEdge(edge_iter->node_l->ptr, edge_iter->node_r->ptr, edge_iter->label);
    edge_iter = edge_iter->next;
  }

  return;
}
