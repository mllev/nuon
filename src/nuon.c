/* NUON - Graph Database
 *
 * For Holly
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

#include "nuon.h"

int height ();
map_node_t* map_node_init (int, const char*, void*);
void map_node_destroy (map_node_t *);

static inline int strncmpsafe (const char* k0, unsigned char* k1)
{
  int kl0, kl1, kl, res;

  kl0 = strlen(k0);
  kl1 = strlen((const char*)k1);
  kl = (kl0 < kl1 ? kl0 : kl1);
  res = strncmp((const char*)k0, (const char*)k1, kl);

  if ( !res ) {
    if ( kl0 == kl1 ) {
      return res;
    } else if ( kl0 < kl1 ) {
      return -1;
    } else {
      return 1;
    }
  } else {
    return res;
  }
}

map_t* map_init ()
{
  map_t* m = malloc(sizeof(map_t));

  if ( m ) {
    int i;
    memset(m, 0, sizeof(map_t));
    m->height = 0;
    m->head = map_node_init(MAX, NULL, NULL);
  }

  return m;
}

/* initialize a node */
map_node_t* map_node_init (int height, const char* k, void* v)
{
  int klen = k ? strlen(k) : 0;
  int h = height;
  int i;
  map_node_t* n = malloc(sizeof(map_node_t));

  if ( n ) {
    if ( k && v ) {
      n->key = malloc(klen + 1);
      n->data = v;
    } else {
      n->key = NULL;
      n->data = NULL;
    }

    if ( n->key ) {
      n->key[klen] = 0; 
      memcpy(n->key, k, klen);
    }

    n->height = h;
    n->next = malloc(sizeof(map_node_t*)*h);
    
    if ( n->next ) {
      for ( i = 0; i < h; i++ ) {
        n->next[i] = NULL;
      }
    }
  }
  return n;
}

int map_remove (map_t* m, const char* k)
{
  int klen = strlen(k);
  int h = m->height;
  int i = 0;
  map_node_t* update[MAX];
  map_node_t* iter = m->head;
  map_node_t* del;

  while ( --h >= 0 ) {
    while ( iter->next[h] && strncmpsafe(k, iter->next[h]->key) < 0 ) {
      iter = iter->next[h];
    }

    update[h] = iter;
  }

  del = update[0]->next[0];

  if ( iter->next[0] && !strncmpsafe(k, iter->next[0]->key) ) {
    h = m->height;

    while ( i < h ) {
      if ( strncmpsafe(k, update[i]->next[i]->key) ) {
        break;
      }

      update[i]->next[i] = update[i]->next[i]->next[i];
      i++;
    }

    while ( m->height > 0 && !m->head->next[m->height-1] ) {
      (m->height)--;
    }

    map_node_destroy(del);
  }
  return 0;
}

int map_set (map_t* m, const char* k, void* v)
{
  int klen = strlen(k);
  int vlen = strlen(v);
  int h = m->height;
  map_node_t* update[MAX];
  map_node_t* iter = m->head;
  map_node_t* n;

  while ( --h >= 0 ) {
    while ( iter->next[h] && strncmpsafe(k, iter->next[h]->key) < 0 ) {
      iter = iter->next[h];
    }

    update[h] = iter;
  }

  if ( iter->next[0] && !strncmpsafe(k, iter->next[0]->key) ) {
    free(iter->next[0]->data);
    iter->next[0]->data = v;
    return 1;
  }

  h = height();

  if (h > m->height) {
    h = ++(m->height);
    update[h-1] = m->head;
  }

  n = map_node_init(h, k, v);

  if ( !n ) {
    return 0;
  }

  while ( --h >= 0 ) {
    n->next[h] = update[h]->next[h];
    update[h]->next[h] = n;
  }

  return 1;
}

void* map_get (map_t* m, const char* k)
{
  map_node_t* iter = m->head;
  int h = m->height;
  int klen = strlen(k);

  while (--h >= 0) {
    while ( iter->next[h] && strncmpsafe(k, iter->next[h]->key) < 0 ) {
      iter = iter->next[h];
    }
  }

  if ( !iter->next[0] || strncmpsafe(k, iter->next[0]->key) != 0 ) {
    return NULL;
  }

  return iter->next[0]->data;
}

void map_iter (map_t* m, void (*on_iter)(map_node_t*))
{
  /* iterate through the map and call a callback on each iteration */
  map_node_t* ptr = m->head;

  while ( ptr ) {
    (*on_iter)(ptr);
    ptr = ptr->next[0];
  }

  return;
}

void map_node_destroy (map_node_t* n)
{
  if ( !n ) {
    return;
  }

  if ( n->key ) {
    free(n->key);
  }

  /* data should be freed by host */

  if ( n->next ) {
    free(n->next);
  }

  free(n);
}

int height ()
{
  static int bits = 0;
  static int reset = 0;

  int h, found = 0;

  for ( h = 0; !found; h++ ) {
    if ( reset == 0 ) {
      bits = rand();
      reset = MAX;
    }

    found = bits & 1;
    bits = bits >> 1;
    --reset;
  }

  if ( h >= MAX ) {
    h = MAX - 1;
  }

  return h;
}

Property* property_init (unsigned char*, unsigned char*);
void property_destroy (Property*);
void vertex_destroy (Vertex*);
Edge* edge_init (Vertex*, Vertex*, unsigned char*);
VertexContainer* graph_vertexContainerInit (Vertex* vertex);

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
  } else {
    if (!strncmpsafe((const char *)iter->key, key)) {
      vertex->properties = property_init(key, val);
      vertex->properties->next = iter->next;
      property_destroy(iter);
      return;
    }
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

#define IS_CREATE_TOK(x) !strncmp((const char*)x, "CREATE", 6) || !strncmp((const char*)x, "create", 6)
#define IS_MATCH_TOK(x) !strncmp((const char*)x, "MATCH", 5) || !strncmp((const char*)x, "match", 5)
#define IS_RETURN_TOK(x) !strncmp((const char*)x, "RETURN", 6) || !strncmp((const char*)x, "return", 6)
#define IS_SET_TOK(x) !strncmp((const char*)x, "SET", 3) || !strncmp((const char*)x, "set", 3)
#define IS_AS_TOK(x) !strncmp((const char*)x, "AS", 2) || !strncmp((const char*)x, "as", 2)

int token_isWhite (unsigned char);
void token_skipWhite (unsigned char**);

int token_isWhite (unsigned char c)
{
  return (c == ' ' || c == '\n' || c == '\t');
}

void token_skipWhite (unsigned char** c)
{
  while ( **c && token_isWhite(**c) ) {
    (*c)++;
  }
}

Token* token (unsigned char** i)
{
  Token* token = malloc(sizeof(Token));
  token->data = NULL;

  token_skipWhite(i);

  switch (**i) {
    case '(':
      (*i)++;
      token->sym = lparen;
      break;
    case ')':
      (*i)++;
      token->sym = rparen; 
      break;
    case '.':
      (*i)++;
      token->sym = period;
      break;
    case '=':
      (*i)++;
      token->sym = equals;
      break;
    case '[':
      (*i)++;
      token->sym = lbrack; 
      break;
    case ']':
      (*i)++;
      token->sym = rbrack; 
      break;
    case '{':
      (*i)++;
      token->sym = lbrace; 
      break;
    case '}':
      (*i)++;
      token->sym = rbrace; 
      break;
    case ':':
      (*i)++;
      token->sym = colon; 
      break;
    case ',':
      (*i)++;
      token->sym = comma;
      break;
    case '-':
      (*i)++;
      token->sym = dash;
      break;
    case '>':
      (*i)++;
      token->sym = grthan;
      break;
    case '"':
      do {
        unsigned char* str = malloc(1024);
        int j = 0;

        (*i)++;

        while ( **i != '"' && j < 1024 ) {
          str[j++] = *((*i)++);

          /* backslashes escape quotes */
          if ( *((*i) - 1) == '\\' ) {
            str[j - 1] = *((*i)++);
          }
        }

        str[j] = 0;
        (*i)++;

        token->sym = string;
        token->data = str;
      } while (0);
      break;
    default:
      if ( IS_CREATE_TOK(*i) ) {
        token->sym = create;
        (*i) += 6;
      } else if ( IS_SET_TOK(*i) ) {
        token->sym = set_sym;
        (*i) += 3;
      } else if ( IS_AS_TOK(*i) ) {
        token->sym = as_sym;
        (*i) += 2;
      } else if ( IS_RETURN_TOK(*i) ) {
        token->sym = return_sym;
        (*i) += 6;
      } else if ( IS_MATCH_TOK(*i) ) {
        token->sym = match;
        (*i) += 5;
      } else if ( (**i >= 65 && **i <= 90) || (**i >= 97 && **i <= 122) ) {
        unsigned char* str = malloc(1024);
        int j = 0;

        while ( ((**i >= 65 && **i <= 90) || (**i >= 97 && **i <= 122)) && j < 1024) {
          str[j++] = *((*i)++);
        }

        str[j] = 0;

        token->sym = ident;
        token->data = str;
      } else {
        token = NULL;
      }
      break;
  }

  return token;
}

typedef struct {
  /* current token */
  Token* tok;

  char cmd[10];

  /* program pointer */
  unsigned char** prog;

  /* current data */
  unsigned char* cache;
  /* previous data */
  unsigned char* prev;

  /* for execution */
  node_data_t *node_root, *node_curr;
  edge_data_t *edge_root, *edge_curr;

  /* updating nodes */
  node_set_data_t *update_node_root, *update_node_curr;
  edge_set_data_t *update_edge_root, *update_edge_curr;

} __Global;

/* for error reporting */
const char* symstr[] = {
  "(",      ")",       "[",
  "]",      "{",       "}",
  ":",      "create",  "match",
  "ident",  "string",  "set",
  ",",      "-",       ">",
  "return", ".",       "=",
  "as"
};

void getsym (__Global*);
void setcmd (__Global*, const char* cmd);
void error (const char *, const char *);

int peek (__Global*, Symbol);
int accept (__Global*, Symbol);
int expect (__Global*, Symbol);

void _create (__Global*);
void _match (__Global*);
void _nodeList (__Global*);
void _matchNodeList (__Global*);
void _node (__Global*);
void _type (__Global*);
void _data (__Global*);
void _keyValueList(__Global*);
void _expr (Graph*, __Global*);
void _identList (__Global*);
void _return (__Global*);
void _set (__Global*);
void _setList (__Global*);
void _property (__Global*);
void _edge (__Global*);

void error (const char* err, const char* s)
{
  fprintf(stderr, "error: %s %s\n", err, s);
  exit(1);
}

void setcmd (__Global* data, const char* cmd)
{
  int len = strlen(cmd);
  strncpy(data->cmd, cmd, len);
  data->cmd[len] = 0;
}

int accept (__Global* data, Symbol s)
{
  if ( data->tok && data->tok->sym == s ) {
    if ( data->tok->data ) {
      if ( data->cache ) {
        data->prev = data->cache;
      }
      data->cache = data->tok->data;
    }
    getsym(data);
    return 1;
  }

  return 0;
}

int peek (__Global* data, Symbol s)
{
  if ( data->tok && data->tok->sym == s ) {
    return 1;
  }

  return 0;
}

int expect (__Global* data, Symbol s)
{
  if ( accept(data, s) ) {
    return 1;
  }

  if ( data->tok ) {
    error("expect: unexpected symbol", symstr[data->tok->sym]);
  } else {
    error("expect: expected symbol", symstr[s]);
  }

  return 0;
}

void _edge (__Global* data) 
{
  expect(data, lbrack);
  expect(data, ident);
  expect(data, rbrack);
  expect(data, dash);
  expect(data, grthan);

  if ( strncmp(data->cmd, "set", 3) ) {
     /***/
    data->edge_curr = exec_addEdge(data->edge_root, NULL);
    exec_addLabelToEdge(data->edge_curr, data->cache);
    /***/

    if ( !data->edge_root ) {
      data->edge_root = data->edge_curr;
    }
  }

  if ( !strncmp(data->cmd, "create", 6) ) {
    /* setCurrentNodeAsLeftNodeToCurrentEdge() */
    exec_setLeftNode(data->node_curr, data->edge_curr);
    /***/
  }
}

void _identList (__Global* data)
{
  expect(data, ident);

  if ( accept(data, period) ) {
    expect(data, ident);
  }

  if ( accept(data, comma) ) {
    _identList(data);
  }
}

void _return (__Global* data)
{
  if ( accept(data, return_sym) ) {
    _identList(data);
  }
}

void _set (__Global* data)
{
  unsigned char *iden, *prop, *val;
  unsigned char *left, *right, *label;

  if ( accept(data, lparen) ) {
    expect(data, ident);
    left = data->cache;
    expect(data, rparen);
    expect(data, dash);
    _edge(data);
    label = data->cache;
    expect(data, lparen);
    expect(data, ident);
    right = data->cache;
    expect(data, rparen);
    data->update_edge_curr = exec_addEdgeUpdate(data->update_edge_root, label, left, right);
    if ( !data->update_edge_root ) {
      data->update_edge_root = data->update_edge_curr;
    }
    return;
  }

  _property(data);

  iden = data->prev;
  prop = data->cache;

  expect(data, equals);
  expect(data, string);

  val = data->cache;

  data->update_node_curr = exec_addNodeUpdate(data->update_node_root, iden, prop, val);

  if ( !data->update_node_root ) {
    data->update_node_root = data->update_node_curr;
  }
}

void _setList (__Global* data)
{
  if ( accept(data, set_sym) ) {
    setcmd(data, "set");
    _set(data);
    _setList(data);
  }
}

void _property (__Global* data)
{
  expect(data, ident);
  expect(data, period);
  expect(data, ident);
}

void _keyValueList (__Global* data)
{
  expect(data, ident);
  expect(data, colon);
  expect(data, string);

  /* addPropertyToCurrentNode(key: data->prev, val: data->cache) */
  exec_addProperty(data->node_curr, data->prev, data->cache);
  /***/

  if ( accept(data, comma) ) {
    _keyValueList(data);
  }
}

void _data (__Global* data)
{
  expect(data, lbrace);
  _keyValueList(data);
  expect(data, rbrace);
}

void _type (__Global* data)
{
  expect(data, ident);

  if ( accept(data, as_sym) ) {
    expect(data, ident);
    /* addNodeAndSetCurrent(ident: data->prev) */
    /* addLabelToCurrent(label: data->cache) */
    data->node_curr = exec_addNode(data->node_root, data->cache);
    exec_addLabelToNode(data->node_curr, data->prev);
    /* because the new node created may be the first node created */
    if ( data->node_curr && !data->node_root ) {
      data->node_root = data->node_curr;
    }
    /***/
    _data(data);
  } else if ( peek(data, lbrace) ) {
    data->node_curr = exec_addNode(data->node_root, NULL);
    exec_addLabelToNode(data->node_curr, data->cache);
    /* because the new node created may be the first node created */
    if ( data->node_curr && !data->node_root ) {
      data->node_root = data->node_curr;
    }
    /***/
    _data(data);
  } else {
    if ( !strncmp(data->cmd, "create", 6) ) {
      /* setCurrentNode(ident: data->cache) */
      data->node_curr = exec_findNode(data->node_root, data->cache);
      if ( !data->node_curr ) {
        error("unidentified variable", (const char *)data->cache);
      }
      /***/
    } else {
      data->node_curr = exec_addNode(data->node_root, data->cache);
      /* because the new node created may be the first node created */
      if ( data->node_curr && !data->node_root ) {
        data->node_root = data->node_curr;
      }
      /***/
    }
  }
}

void _node (__Global* data)
{
  expect(data, lparen);
  _type(data);
  expect(data, rparen);
}

void _nodeList (__Global* data)
{
  _node(data);

  if ( accept(data, comma) ) {
    _nodeList(data);
    return;
  }

  if ( !accept(data, dash) ) {
    return;
  }

  _edge(data);
  _node(data);

  /* setCurrentNodeAsRightNodeToCurrentEdge() */
  exec_setRightNode(data->node_curr, data->edge_curr);
  /***/

  if ( accept(data, comma) ) {
    _nodeList(data);
  }
}

void _matchNodeList (__Global* data)
{
  _node(data);

  if ( accept(data, comma) ) {
    _matchNodeList(data);
    return;
  }
}

void _create (__Global* data)
{
  setcmd(data, "create");
  _nodeList(data);
}

void _match (__Global* data)
{
  setcmd(data, "match");
  _matchNodeList(data);
}

void _expr (Graph* g, __Global* data)
{
  if ( accept(data, create) ) {
    _create(data);
    exec_cmd(g, data->cmd, data->node_root, data->edge_root, data->update_node_root, data->update_edge_root);
  }

  else if ( accept(data, match) ) {
    _match(data);
    exec_cmd(g, data->cmd, data->node_root, data->edge_root, data->update_node_root, data->update_edge_root);
    _setList(data);
    if ( strncmp(data->cmd, "match", 5 ) ) {
      exec_cmd(g, data->cmd, data->node_root, data->edge_root, data->update_node_root, data->update_edge_root);
    }
    _return(data);
  }

  else if ( data->tok && data->tok->data ) {
    fprintf(stderr, "error: unknown command %s\n", (const char *)data->tok->data);
  }
}

void getsym (__Global* data)
{
  if ( data->tok ) {
    free(data->tok);
  }
  data->tok = token(data->prog);
}

void parse (Graph* g, unsigned char* p) 
{
  __Global data;

  data.prog = &p;
  data.tok = NULL;
  data.cache = NULL;
  data.prev = NULL;
  data.node_root = NULL;
  data.node_curr = NULL;
  data.edge_root = NULL;
  data.edge_curr = NULL;
  data.update_node_root = NULL;
  data.update_node_curr = NULL;
  data.update_edge_root = NULL;
  data.update_edge_curr = NULL;

  memset(data.cmd, 0, 10);

  getsym(&data);
  _expr(g, &data);
}

static inline int strlensafe (unsigned char *str)
{
  int l = strlen((const char *)str);
  return (l > 512 ? 512 : l);
}

node_data_t* exec_addNode(node_data_t* root, unsigned char* ident) 
{
  int len;
  node_data_t* node;

  node = malloc(sizeof(node_data_t));

  if ( ident ) { 
    len = strlensafe(ident);
    strncpy((char *)node->ident, (char *)ident, len);
    node->ident[len] = 0;
  } else {
    node->ident[0] = 0;
  }

  node->next = NULL;

  if ( root ) {
    while ( root->next ) { 
      if ( ident && !strncmp((const char *)root->ident, (const char *)ident, len) ) {
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

void exec_printData (VertexContainer *vertices, int newline)
{
  Property* prop_iter;
  Edge* edge_iter;
  VertexContainer* vc_iter, *cont;
  vc_iter = vertices;

  while ( vc_iter ) {
    printf("{");
    prop_iter = vc_iter->vertex->properties;
    edge_iter = vc_iter->vertex->edges;
    while ( prop_iter ) {
      printf("%s:\"%s\"", prop_iter->key, prop_iter->val);
      if ( prop_iter->next ) {
        printf(",");
      }
      prop_iter = prop_iter->next;
    }
    if ( edge_iter ) {
      printf(",");
    }
    while ( edge_iter ) {
      printf("%s:", edge_iter->label);
      cont = malloc(sizeof(VertexContainer));
      cont->vertex = edge_iter->to;
      cont->next = NULL;
      //vc_iter = cont;
      exec_printData(cont, 0);
      if ( edge_iter->next ) {
        printf(",");
      }
      edge_iter = edge_iter->next;
    }
    printf("}");

    if (newline) {
      printf("\n");
    }

    vc_iter = vc_iter->next;
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
        exec_printData(node_iter->vrtxdata, 1);
      } else {
        while (count) {
          count--;
          node_iter->vrtxdata = graph_getVertices(g, node_iter->label, node_iter->keys[count], node_iter->vals[count]);
          exec_printData(node_iter->vrtxdata, 1);
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
              if (leftData->vertex != rightData->vertex)
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
    if (edge_iter->node_l->ptr != edge_iter->node_r->ptr)
      graph_vertexAddEdge(edge_iter->node_l->ptr, edge_iter->node_r->ptr, edge_iter->label);
    edge_iter = edge_iter->next;
  }

  return;
}
