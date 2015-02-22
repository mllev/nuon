/* parser.c
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

#include "token.h"
#include "parser.h"
#include "exec.h"

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

  if ( accept(data, ident) ) {
    if ( strncmp(data->cmd, "set", 3) ) {
      /* createEdgeAndSetCurrent(ident: data->cache) */
      data->edge_curr = exec_addEdge(data->edge_root, data->cache);
      /***/
    }
  } else {
    if ( strncmp(data->cmd, "set", 3) ) {
      /* createEdgeAndSetCurrent(ident: null) */
      data->edge_curr = exec_addEdge(data->edge_root, NULL);
      /***/
    }
  }

  /* because the new edge created may be the first edge created */
  if ( data->edge_curr && !data->edge_root ) {
    data->edge_root = data->edge_curr;
  }
  /***/

  expect(data, colon);
  expect(data, ident);
  if ( strncmp(data->cmd, "set", 3) ) {
     /* setLabelOnCurrentEdge(label: data->cache) */
    exec_addLabelToEdge(data->edge_curr, data->cache);
    /***/
  }
  expect(data, rbrack);
  expect(data, dash);
  expect(data, grthan);

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
  expect(data, string);
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

void _create (__Global* data)
{
  setcmd(data, "create");
  _nodeList(data);
}

void _match (__Global* data)
{
  setcmd(data, "match");
  _nodeList(data);
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