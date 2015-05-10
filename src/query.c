/* query.c
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

#include "query.h"

void nuonNextToken (nuonState* state)
{
  unsigned char** i = state->prog;
  state->tok = malloc(sizeof(nuonToken));

  if (!state->tok) {
    state->tok = NULL;
    return;
  }

  state->tok->data = NULL;

  while (**i && (**i == ' ' || **i == '\n' || **i == '\t')) {
    (*i)++;
  }

  switch (**i) {
    case '.':
      (*i)++;
      state->tok->sym = period;
      break;
    case '=':
      (*i)++;
      state->tok->sym = equals;
      break;
    case ',':
      (*i)++;
      state->tok->sym = comma;
      break;
    case '?':
      (*i)++;
      state->tok->sym = qmark;
      break;
    case '"':
      do {
        unsigned char* str = malloc(1024);
        int j = 0;

        (*i)++;

        while ( **i != '"' && j < 1024 ) {
          str[j++] = *((*i)++);
          if ( *((*i) - 1) == '\\' ) {
            str[j - 1] = *((*i)++);
          }
        }

        str[j] = 0;
        (*i)++;
        state->tok->sym = string;
        state->tok->data = str;
      } while (0);
      break;
    default:
      if (NUON_IS_CREATE_SYM(*i)) {
        state->tok->sym = create_sym;
        (*i) += 6;
      } else if (NUON_IS_SET_SYM(*i)) {
        state->tok->sym = set_sym;
        (*i) += 3;
      } else if (NUON_IS_SELECT_SYM(*i)) {
        state->tok->sym = select_sym;
        (*i) += 6;
      } else if (NUON_IS_RETURN_SYM(*i)) {
        state->tok->sym = return_sym;
        (*i) += 6;
      } else if (NUON_IS_WHERE_SYM(*i)) {
        state->tok->sym = where_sym;
        (*i) += 5;
      } else if (NUON_IS_AND_SYM(*i)) {
        state->tok->sym = and_sym;
        (*i) += 3;
      } else if (NUON_IS_NODE_SYM(*i)) {
        state->tok->sym = node_sym;
        (*i) += 4;
      } else if (NUON_IS_ARROW_SYM(*i)) {
        state->tok->sym = arrow;
        (*i) += 2;
      } else if ((**i >= 65 && **i <= 90) || (**i >= 97 && **i <= 122)) {
        unsigned char* str = malloc(1024);
        int j = 0;

        while ( ((**i >= 65 && **i <= 90) || (**i >= 97 && **i <= 122)) && j < 1024) {
          str[j++] = *((*i)++);
        }

        str[j] = 0;
        state->tok->sym = ident;
        state->tok->data = str;
      } else {
        free(state->tok);
        state->tok = NULL;
      }
      break;
  }
}

void nuonParseError (nuonState* state, const char* err, const char* s)
{
  fprintf(stderr, "error: %s : %s\n", err, s);
  state->err = 1;
  return;
}

int nuonAccept (nuonState* state, nuonSymbol s)
{
  if (state->err) { return 0; }
  if (state->tok && state->tok->sym == s)  {
    nuonGetSym(state);
    return 1;
  }

  return 0;
}

int nuonExpect (nuonState* state, nuonSymbol s)
{
  if (state->err) { return 0; }
  if (nuonAccept(state, s)) { return 1; }

  if (state->tok) {
    nuonParseError(state, "unexpected symbol", nuonSymstr[state->tok->sym]);
  } else {
    nuonParseError(state,"expected symbol", nuonSymstr[s]);
  }

  return 0;
}

int nuonPeek (nuonState* state, nuonSymbol s)
{
  if (state->err) { return 0; }
  if (state->tok && state->tok->sym == s) {
    return 1;
  }

  return 0;
}

void nuonGetSym (nuonState* state)
{
  NUON_HANDLE_ERR(state);

  if (state->tok) {
    if (state->tok->data) {
      free(state->tok->data);
    }
    free(state->tok);
    state->tok = NULL;
  }
  nuonNextToken(state);
}

void nuonParseNode (nuonState* state)
{
  NUON_HANDLE_ERR(state);

  if (!nuonAccept(state, ident)) {
    nuonExpect(state, node_sym);
  }
  nuonExpect(state, ident);
}

void nuonParseProperty (nuonState* state)
{
  NUON_HANDLE_ERR(state);

  nuonExpect(state, ident);
  if (!nuonAccept(state, period)) {
    nuonExpect(state, arrow);
  }
  nuonExpect(state, ident);
  nuonExpect(state, equals);
  nuonExpect(state, string);
}

void nuonParseSetPropertyList (nuonState* state)
{
  NUON_HANDLE_ERR(state);

  nuonParseProperty(state);
  if (nuonAccept(state, comma)) {
    nuonParseSetPropertyList(state);
  }
}

void nuonParseSet (nuonState* state)
{
  NUON_HANDLE_ERR(state);

  nuonExpect(state, set_sym);
  nuonParseSetPropertyList(state);
}

void nuonParseCreateNodeList (nuonState* state)
{
  NUON_HANDLE_ERR(state);

  nuonParseNode(state);
  if (nuonAccept(state, comma)) {
    nuonParseCreateNodeList(state);
  }
}

void nuonParseCreate (nuonState* state)
{
  NUON_HANDLE_ERR(state);

  nuonExpect(state, create_sym);
  nuonParseCreateNodeList(state);
  if (nuonPeek(state, set_sym)) {
    nuonParseSet(state);
  }
}

void nuonParse (unsigned char** prog)
{
  nuonState* state = malloc(sizeof(nuonState));

  state->prog = prog;
  state->err = 0;
  state->tok = NULL;
  nuonGetSym(state);

  if (nuonPeek(state, create_sym)) {
    nuonParseCreate(state);
  }

  free(state);
}

unsigned char* nuonReadLine (FILE* f)
{
  unsigned char* buf = NULL;
  int c = 0, i = 0, bufsize = 10;

  buf = malloc(bufsize + 1);
  memset(buf, 0, bufsize + 1);

  while ((c = fgetc(f)) != EOF) {
    if (i == bufsize) {
      buf = realloc(buf, (bufsize += 10) + 1);
    }
    buf[i++] = (unsigned char)c;
    if (buf[i - 1] == '\n') {
      break;
    }
  }

  buf[i] = 0;
  return buf;
}

int main (void)
{
  unsigned char *line, *l;

  while (1) {
    printf("nuon> ");
    l = line = nuonReadLine((FILE* )stdin);
    nuonParse(&line);
    free(l);
  } 
}