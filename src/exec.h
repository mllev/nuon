/* exec.h
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

#ifndef _EXEC_H
#define _EXEC_H

#include "graph.h"

typedef struct node_data node_data_t;
typedef struct edge_data edge_data_t;
typedef struct node_set_data node_set_data_t;
typedef struct edge_set_data edge_set_data_t;

struct node_data {
  /* identifier */
  unsigned char ident[512];

  /* node label */
  unsigned char label[512];

  /* properties */
  unsigned char keys[20][512];
  unsigned char vals[20][512];

  int propcount;

  /* linked list */
  node_data_t* next;

  /* pointer to node in graph */
  Vertex* ptr;

  VertexContainer *vrtxdata;
};

struct edge_data {
  /* identifier */
  unsigned char ident[512];

  /* edge label */
  unsigned char label[512];

  node_data_t *node_l, *node_r;

  /* linked list */
  edge_data_t* next;
};

struct node_set_data {
  unsigned char ident[512];
  unsigned char prop[512];
  unsigned char val[512];
  node_set_data_t* next;
};

struct edge_set_data {
  unsigned char left[512];
  unsigned char right[512];
  unsigned char label[512];
  edge_set_data_t* next;
};

/* node api */
node_data_t* exec_addNode(node_data_t*, unsigned char*);
void exec_addLabelToNode(node_data_t*, unsigned char*);
node_data_t* exec_findNode(node_data_t*, unsigned char*);
void exec_addProperty(node_data_t*, unsigned char*, unsigned char*);

/* node updating api */
node_set_data_t* exec_addNodeUpdate(node_set_data_t*, unsigned char*, unsigned char*, unsigned char*);
edge_set_data_t* exec_addEdgeUpdate(
  edge_set_data_t*, 
  unsigned char*, 
  unsigned char*, 
  unsigned char*
);

/* relationship api */
edge_data_t* exec_addEdge(edge_data_t*, unsigned char*);
void exec_setRightNode(node_data_t*, edge_data_t*);
void exec_setLeftNode(node_data_t*, edge_data_t*);
void exec_addLabelToEdge(edge_data_t*, unsigned char*);

/* execute commands */
void exec_cmd (Graph*, char*, node_data_t*, edge_data_t*, node_set_data_t*, edge_set_data_t*);
void exec_printData (VertexContainer*, int);

#endif