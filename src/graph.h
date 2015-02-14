/* graph.h
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

#ifndef _GRAPH_H
#define _GRAPH_H

#include "map.h"

/* for portability */
#define uint64 unsigned long long

typedef struct graph Graph;
typedef struct vertex Vertex;
typedef struct property Property;
typedef struct edge Edge;

typedef unsigned long word_t;

struct graph {
  map_t* vertices;
};

struct vertex {
  word_t idx;
  Edge* edges;
  Property* properties;
};

struct edge {
  void* label;
  int ll;
  Vertex* to;
  Vertex* from;
  Edge* next;
};

struct property {
  void* key;
  void* val;
  int kl;
  int vl;
  Property* next;
};

Graph* graph_init (uint64);
Vertex* graph_setVertex (Graph*, void*, Vertex*);
Vertex* graph_getVertex (Graph*, void*);
Vertex* graph_vertexInit (void);
void graph_removeVertex (Graph*, void*);
void graph_vertexAddEdge (Vertex*, Vertex*, void*);
void graph_vertexRemoveEdge (Vertex*, void*);
void graph_vertexSetProperty (Vertex*, void*, void*);
void* graph_vertexGetProperty (Vertex*, void*);
void graph_vertexRemoveProperty (Vertex*, void*);

#endif