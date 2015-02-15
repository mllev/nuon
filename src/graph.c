/* graph.c
 *
 * Copyright (c) 2015, Matthew Levenstein
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the name of Redis nor the names of its contributors may be used
 *      to endorse or promote products derived from this software without
 *      specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "graph.h"

Property* property_init (unsigned char*, unsigned char*);
void property_destroy (Property*);
void vertex_destroy (Vertex*);
Edge* edge_init (Vertex*, Vertex*, unsigned char*);
VertexContainer* graph_vertexContainerInit (Vertex* vertex);

static inline int strncmpsafe (const char* k0, unsigned char* k1)
{
  int kl0, kl1, kl;

  kl0 = strlen(k0);
  kl1 = strlen((const char*)k1);
  kl = (kl0 > kl1 ? kl0 : kl1);
  
  return strncmp((const char*)k0, (const char*)k1, kl);
}

Graph* graph_init (uint64 size)
{
  Graph* g = malloc(sizeof(Graph));

  if ( !g ) {
    return NULL;
  }

  g->vertices = map_init();

  if ( !g->vertices ) {
    free(g);
    return NULL;
  }

  return g;
}

Vertex* graph_vertexInit (void)
{
  Vertex* v = malloc(sizeof(Vertex));

  if ( !v ) {
    return NULL;
  }

  v->idx = 0;
  v->edges = NULL;
  v->properties = NULL;

  return v;
}

Property* property_init (unsigned char* key, unsigned char* val)
{
  Property* p = malloc(sizeof(Property));
  int kl = strlen((const char *)key);
  int vl = strlen((const char *)val);

  if ( !p ) {
    return NULL;
  }

  p->key = malloc(kl + 1);

  if ( !p->key ) {
    free(p);
    return NULL;
  }

  p->val = malloc(vl + 1);

  if ( !p->val ) {
    free(p->key);
    free(p);
    return NULL;
  }

  strncpy((char *)p->key, (char *)key, kl);
  strncpy((char *)p->val, (char *)val, vl);

  p->key[kl] = 0;
  p->val[vl] = 0;

  p->next = NULL;

  return p;
}

Edge* edge_init (Vertex* from, Vertex* to, unsigned char* label)
{
  Edge* e = malloc(sizeof(Edge));
  int len = strlen((const char *)label);

  if ( !e ) {
    return NULL;
  }

  e->label = malloc(len + 1);

  if ( !e->label ) {
    free(e);
    return NULL;
  }

  strncpy((char *)e->label, (char *)label, len);
  e->label[len] = 0;

  e->from = from;
  e->to = to;
  e->next = NULL;

  return e;
}

void property_destroy (Property* property)
{
  if ( !property ) {
    return;
  }

  if (property->key) {
    free(property->key);
  }

  if (property->val) {
    free(property->val);
  }

  free(property);
}

void vertex_destroy (Vertex* vertex)
{
  Property* piter;
  Edge* eiter;
  Property* pdel = NULL;
  Edge* edel = NULL;

  if ( !vertex ) {
    return;
  }

  piter = vertex->properties;
  eiter = vertex->edges;

  while (piter) {
    pdel = piter;
    piter = piter->next;
    property_destroy(pdel);
  }

  while (eiter) {
    edel = eiter;
    eiter = eiter->next;
    free(edel);
  }

  free(vertex);
}

Vertex* graph_setVertex (Graph* graph, unsigned char* key, Vertex* vertex)
{
  Vertex* v;

  if ( vertex ) {
    v = vertex;
  } else {
    v = graph_vertexInit();
  }

  map_set(graph->vertices, (const char *)key, v);

  return v;
}

Vertex* graph_getVertex (Graph* graph, unsigned char* key)
{
  Vertex* v = (Vertex *)map_get(graph->vertices, (const char *)key);
  
  if ( v ) {
    return v;
  }

  return NULL;
}

void graph_removeVertex (Graph* graph, unsigned char* key)
{
  map_remove(graph->vertices, (const char*)key);
}

void graph_vertexAddEdge (Vertex* from, Vertex* to, unsigned char* label)
{
  Edge* iter = from->edges;

  if ( !iter ) {
    from->edges = edge_init(from, to, label);
    return;
  }

  while ( iter->next ) {
    iter = iter->next;
  }

  iter->next = edge_init(from, to, label);
  return;
}

void graph_vertexRemoveEdge (Vertex* vertex, unsigned char* label)
{
  /* remove either a single instance of the edge, or all the instances */
  /* maybe give the option, or have the user specify a count */
  return;
}

void graph_vertexSetProperty (Vertex* vertex, unsigned char* key, unsigned char* val)
{
  Property* iter = vertex->properties;
  int exists = 0;
  int kl = strlen((const char *)key);

  if ( !iter ) {
    vertex->properties = property_init(key, val);
    return;
  }

  while (iter->next) {
    if (!strncmpsafe((const char *)iter->next->key, key)) {
      exists = 1;
      break;
    }
    iter = iter->next;
  }

  if (exists) {
    Property* del = iter->next;
    iter->next = del->next;
    property_destroy(del);
  }

  iter->next = property_init(key, val);

  return;
}

unsigned char* graph_vertexGetProperty (Vertex* vertex, unsigned char* key)
{
  Property* iter = vertex->properties;
  int kl = strlen((const char *)key);

  while ( iter ) {
    if (!strncmpsafe((const char *)iter->key, key)) {
      break;
    }
    iter = iter->next;
  }

  if ( iter ) {
    return iter->val;
  }

  return NULL;
}

void graph_vertexRemoveProperty (Vertex* vertex, unsigned char* key)
{
  Property *iter, *del = NULL;
  int kl = strlen((const char *)key);

  if ( !vertex->properties ) {
    return;
  }

  iter = vertex->properties;

  /* if it's the first one to be deleted */
  if ( !strncmpsafe((const char *)iter->key, key)) {
    del = iter;
    vertex->properties = iter->next;
    property_destroy(del);
    return;
  }

  while ( iter->next ) {
    if ( !strncmpsafe((const char*)iter->next->key, key) ) {
      break;
    }
    iter = iter->next;
  }

  if ( iter->next ) {
    del = iter->next;
    iter->next = del->next;
    property_destroy(del);
  }

  return;
}

VertexContainer* graph_vertexContainerInit (Vertex* vertex)
{
  if ( !vertex->properties ) {
    return NULL;
  }

  VertexContainer* v = malloc(sizeof(VertexContainer));
  v->vertex = vertex;
  v->next = NULL;
  return v;
}

VertexContainer* graph_getVertices (Graph* g, unsigned char* label, unsigned char* key, unsigned char* val)
{
  VertexContainer *head = NULL, *tail = NULL;
  map_node_t* ptr = g->vertices->head;
  Vertex* type;
  Edge* edge_iter;
  unsigned char* prop = NULL;

  if ( label ) {
    type = graph_getVertex(g, label);
    if ( !type ) {
      return NULL;
    }
    edge_iter = type->edges;
    while ( edge_iter ) {
      if ( prop ) {
        prop = NULL;
      }
      if ( key ) {
        prop = graph_vertexGetProperty(edge_iter->to, key);
      }
      if ( !key || (prop && !strncmp((const char *)prop, (const char*)val, strlen((const char *)val))) ) {
        if ( !head ) {
          head = graph_vertexContainerInit(edge_iter->to);
          tail = head;
        } else {
          tail->next = graph_vertexContainerInit(edge_iter->to);
          tail = tail->next;
        }
      }
      edge_iter = edge_iter->next;
    } 
  } else {
    while ( ptr ) {
      if ( prop ) {
        prop = NULL;
      }
      if ( ptr->data ) {
        if ( key ) {
          prop = graph_vertexGetProperty(ptr->data, key);
        }
        if ( !key || (prop && !strncmp((const char *)prop, (const char*)val, strlen((const char *)val))) ) {
          if ( !head ) {
            head = graph_vertexContainerInit(ptr->data);
            tail = head;
          } else {
            tail->next = graph_vertexContainerInit(ptr->data);
            tail = tail->next;
          }
        }
      }
      ptr = ptr->next[0];
    }
  } 

  return head;
}
