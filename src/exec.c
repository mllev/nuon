#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "exec.h"
#include "graph.h"

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
    while ( root->next ) { root = root->next; }
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

void exec_create (node_data_t* root, edge_data_t* edges)
{
  node_data_t* node_iter = root;
  edge_data_t* edge_iter = edges;
  int count = 0;

  while ( node_iter ) { 
    printf("---------------------------------\n");
    printf("New node: %s\n", node_iter->label);
    count = node_iter->propcount;
    while (count) {
      count--;
      printf("\tproperty: %s, %s\n", node_iter->keys[count], node_iter->vals[count]);
    }
    printf("\n");
    node_iter = node_iter->next;
  }

  while ( edge_iter ) { 
    printf("---------------------------------\n");
    printf("New edge: %s\n", edge_iter->label);
    printf("\tleft node: %s\n", edge_iter->node_l->label);
    printf("\tright node: %s\n", edge_iter->node_r->label);
    edge_iter = edge_iter->next;
    printf("\n");
  }
  return;
}

void exec_match (node_data_t* root, edge_data_t* edges)
{
  return;
}
