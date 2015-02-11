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

Property* property_init (void*, void*);
void property_destroy (Property*);
void vertex_destroy (Vertex*);
Edge* edge_init (Vertex*, Vertex*, void*);
Vertex* vertex_dup (Vertex*);


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

  v->edges = NULL;
  v->properties = NULL;

  return v;
}

Vertex* vertex_dup (Vertex* vertex)
{
  Vertex* v = malloc(sizeof(Vertex));

  if ( !v ) {
    return NULL;
  }

  v->edges = vertex->edges;
  v->properties = vertex->properties;

  return v;
}

Property* property_init (void* key, void* val)
{
  Property* p = malloc(sizeof(Property));
  int kl = strlen((char *)key);
  int vl = strlen((char *)val);

  if ( !p ) {
    return NULL;
  }

  p->key = malloc(kl);

  if ( !p->key ) {
    free(p);
    return NULL;
  }

  p->val = malloc(vl);

  if ( !p->val ) {
    free(p->key);
    free(p);
    return NULL;
  }

  strncpy(p->key, key, kl);
  strncpy(p->val, val, vl);

  p->kl = kl;
  p->vl = vl;

  p->next = NULL;

  return p;
}

Edge* edge_init (Vertex* from, Vertex* to, void* label)
{
  Edge* e = malloc(sizeof(Edge));
  int ll = strlen((char *)label);

  if ( !e ) {
    return NULL;
  }

  e->label = malloc(ll);

  if ( !e->label ) {
    free(e);
    return NULL;
  }

  strncpy(e->label, label, ll);
  
  e->ll = ll;
  e->from = from;
  e->to = to;

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

Vertex* graph_setVertex (Graph* graph, void* key, Vertex* vertex)
{
  Vertex* v;

  if ( vertex ) {
    v = vertex;
  } else {
    v = graph_vertexInit();
  }

  map_set(graph->vertices, key, v);

  return v;
}

Vertex* graph_getVertex (Graph* graph, void* key)
{
  Vertex* v = (Vertex *)map_get(graph->vertices, key);
  
  if ( v ) {
    return v;
  }

  return NULL;
}

void graph_removeVertex (Graph* graph, void* key)
{
  map_remove(graph->vertices, key);
}

void graph_vertexAddEdge (Vertex* from, Vertex* to, void* label)
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

void graph_vertexRemoveEdge (Vertex* vertex, void* label)
{
  /* remove either a single instance of the edge, or all the instances */
  /* maybe give the option, or have the user specify a count */
  return;
}

void graph_vertexSetProperty (Vertex* vertex, void* key, void* val)
{
  Property* iter = vertex->properties;
  int exists = 0;

  if ( !iter ) {
    vertex->properties = property_init(key, val);
    return;
  }

  while (iter->next) {
    if (!cmp(iter->next->key, key, iter->next->kl)) {
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

void* graph_vertexGetProperty (Vertex* vertex, void* key)
{
  Property* iter = vertex->properties;

  while ( iter ) {
    if (!cmp(iter->key, key, iter->kl)) {
      break;
    }
    iter = iter->next;
  }

  if ( iter ) {
    return iter->val;
  }

  return NULL;
}

void graph_vertexRemoveProperty (Vertex* vertex, void* key)
{
  Property *iter, *del = NULL;

  if ( !vertex->properties ) {
    return;
  }

  iter = vertex->properties;

  /* if it's the first one to be deleted */
  if ( !cmp(iter->key, key, iter->kl)) {
    del = iter;
    vertex->properties = iter->next;
    property_destroy(del);
    return;
  }

  while ( iter->next ) {
    if ( !cmp(iter->next->key, key, iter->kl) ) {
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
