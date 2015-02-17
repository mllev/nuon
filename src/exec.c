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

node_set_data_t* exec_addNodeUpdate(node_set_data_t* root, unsigned char* ident, unsigned char* key, unsigned char* value)
{
  int ilen, klen, vlen;
  node_set_data_t* node;

  ilen = strlensafe(ident);
  klen = strlensafe(key);
  vlen = strlensafe(value);

  node = malloc(sizeof(node_set_data_t));

  strncpy((char *)node->ident, (char *)ident, ilen);
  strncpy((char *)node->prop, (char *)key, klen);
  strncpy((char *)node->val, (char *)value, vlen);

  node->ident[ilen] = 0;
  node->prop[klen] = 0;
  node->val[vlen] = 0;

  node->next = NULL;

  if ( root ) {
    while ( root->next ) { root = root->next; }
    root->next = node;
  }

  return node;
}

edge_set_data_t* exec_addEdgeUpdate(
  edge_set_data_t* root, 
  unsigned char* label, 
  unsigned char* left, 
  unsigned char* right
){
  int len, llen, rlen;
  edge_set_data_t* edge;

  llen = strlensafe(left);
  rlen = strlensafe(right);
  len = strlensafe(label);

  edge = malloc(sizeof(edge_set_data_t));

  strncpy((char *)edge->label, (char *)label, len);
  strncpy((char *)edge->left, (char *)left, llen);
  strncpy((char *)edge->right, (char *)right, rlen);

  edge->left[llen] = 0;
  edge->right[rlen] = 0;
  edge->label[len] = 0;

  edge->next = NULL;

  if ( root ) {
    while ( root->next ) { root = root->next; }
    root->next = edge;
  }

  return edge;
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

void exec_printData (VertexContainer *vertices)
{
  Property* prop_iter;

  while ( vertices ) {
    printf("{\n");
    prop_iter = vertices->vertex->properties;
    while ( prop_iter ) {
      printf("  %s : %s", prop_iter->key, prop_iter->val);
      if ( prop_iter->next ) {
        printf(",");
      }
      printf("\n");
      prop_iter = prop_iter->next;
    }
    printf("}");
    if ( vertices->next ) {
      printf(",\n\n");
    } else {
      printf("\n\n");
    }
    vertices = vertices->next;
  }
}

void exec_cmd (Graph* g, char* cmd, node_data_t* root, edge_data_t* edges, node_set_data_t* uroot, edge_set_data_t* eroot)
{
  Vertex *type, *node;
  VertexContainer *returnData, *leftData, *rightData;
  node_data_t* node_iter = root;
  node_set_data_t* node_set_iter = uroot;
  edge_set_data_t* edge_set_iter = eroot;
  edge_data_t* edge_iter = edges;
  int count = 0, id;

  if ( !strncmp(cmd, "match", 5) ) {
    while ( node_iter ) {
      count = node_iter->propcount;
      if ( !count ) {
        node_iter->vrtxdata = graph_getVertices(g, NULL, NULL, NULL);
        exec_printData(node_iter->vrtxdata);
      } else {
        while (count) {
          count--;
          node_iter->vrtxdata = graph_getVertices(g, node_iter->label, node_iter->keys[count], node_iter->vals[count]);
          exec_printData(node_iter->vrtxdata);
        }
      }
      node_iter = node_iter->next;
    }
    return;
  }

  if ( !strncmp(cmd, "set", 3) ) {
    while ( edge_set_iter ) { 
      node_iter = root;
      while ( node_iter ) {
        leftData = NULL;
        rightData = NULL;
        if ( !strncmp((const char *)node_iter->ident, 
          (const char *)edge_set_iter->left, 
          strlen((const char*)edge_set_iter->left)) ) {
          leftData = node_iter->vrtxdata;
        }
        if ( !strncmp((const char *)node_iter->ident, 
          (const char *)edge_set_iter->right, 
          strlen((const char*)edge_set_iter->right)) ) {
          rightData = node_iter->vrtxdata;
        }
        if ( leftData && rightData ) {
          while ( leftData ) {
            while ( rightData ) {
              graph_vertexAddEdge(leftData->vertex, rightData->vertex, edge_set_iter->label);
              rightData = rightData->next;
            }
            leftData = leftData->next;
          }
        }
        node_iter = node_iter->next;
      }
      edge_set_iter = edge_set_iter->next;
    }

    while ( node_set_iter ) {
      node_iter = root;
      while ( node_iter ) {
        if ( !strncmp((const char *)node_iter->ident, 
          (const char *)node_set_iter->ident, 
          strlen((const char*)node_iter->ident)) ) {
          returnData = node_iter->vrtxdata;
          while ( returnData ) {
            graph_vertexSetProperty(returnData->vertex, node_set_iter->prop, node_set_iter->val);
            returnData = returnData->next;
          }
        }
        node_iter = node_iter->next;
      }
      node_set_iter = node_set_iter->next;
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
