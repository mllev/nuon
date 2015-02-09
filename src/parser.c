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

typedef struct {
  /* current token */
  Token* tok;

  /* program pointer */
  unsigned char** prog;

  /* current data */
  unsigned char* cache;

} __Global;

/* for error reporting */
const char* symstr[] = {
  "(",      ")",       "[",
  "]",      "{",       "}",
  ":",      "create",  "match",
  "ident",  "string",  "set",
  ",",      "-",       ">",
  "return", ".",       "="
};

void getsym (__Global*);
void error (const char *, const char *);

void _create (__Global*);
void _match (__Global*);
void _nodeList (__Global*);
void _node (__Global*);
void _type (__Global*);
void _data (__Global*);
void _keyValueList(__Global*);
void _expr (__Global*);
void _identList (__Global*);
void _return (__Global*);
void _set (__Global*);
void _setList (__Global*);
void _property (__Global* data);

void error (const char* err, const char* s)
{
  fprintf(stderr, "error: %s %s\n", err, s);
  exit(1);
}

int accept (__Global* data, Symbol s)
{
  if ( data->tok && data->tok->sym == s ) {
    if ( data->tok->data ) {
      data->cache = data->tok->data;
    }
    getsym(data);
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
  _property(data);
  expect(data, equals);
  expect(data, string);
}

void _setList (__Global* data)
{
  if ( accept(data, set_sym) ) {
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

  if ( accept(data, colon) ) {
    expect(data, ident);
    _data(data);
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
  }

  if ( !accept(data, dash) ) {
    return;
  }

  expect(data, lbrack);
  expect(data, ident);
  expect(data, rbrack);
  expect(data, dash);
  expect(data, grthan);

  _node(data);

  if ( accept(data, comma) ) {
    _nodeList(data);
  }
}

void _create (__Global* data)
{
  _nodeList(data);
}

void _match (__Global* data)
{
  _nodeList(data);
  _setList(data);
  _return(data);
}

void _expr (__Global* data)
{
  if ( accept(data, create) ) {
    _create(data);
    _expr(data); /* comment this out to eliminate injection */
  }

  else if ( accept(data, match) ) {
    _match(data);
    _expr(data); /* this too */
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

void parse (unsigned char** p) 
{
  __Global data;

  data.prog = p;
  data.tok = NULL;
  data.cache = NULL;

  getsym(&data);
  _expr(&data);
}