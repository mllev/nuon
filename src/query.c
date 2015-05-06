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

nuonToken_t* nuonNextToken (unsigned char** i)
{
  nuonToken_t* tok = malloc(sizeof(nuonToken_t));
  tok->data = NULL;

  while (**i && (**i == ' ' || **i == '\n' || **i == '\t')) {
    (*i)++;
  }

  switch (**i) {
    case '.':
      (*i)++;
      tok->sym = period;
      break;
    case '=':
      (*i)++;
      tok->sym = equals;
      break;
    case ',':
      (*i)++;
      tok->sym = comma;
      break;
    case '?':
      (*i)++;
      tok->sym = qmark;
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
        tok->sym = string;
        tok->data = str;
      } while (0);
      break;
    default:
      if (NUON_IS_CREATE_SYM(*i)) {
        tok->sym = create_sym;
        (*i) += 6;
      } else if (NUON_IS_SET_SYM(*i)) {
        tok->sym = set_sym;
        (*i) += 3;
      } else if (NUON_IS_SELECT_SYM(*i)) {
        tok->sym = select_sym;
        (*i) += 6;
      } else if (NUON_IS_RETURN_SYM(*i)) {
        tok->sym = return_sym;
        (*i) += 6;
      } else if (NUON_IS_WHERE_SYM(*i)) {
        tok->sym = where_sym;
        (*i) += 5;
      } else if (NUON_IS_AND_SYM(*i)) {
        tok->sym = and_sym;
        (*i) += 3;
      } else if (NUON_IS_NODE_SYM(*i)) {
        tok->sym = node_sym;
        (*i) += 4;
      } else if (NUON_IS_ARROW_SYM(*i)) {
        tok->sym = arrow;
        (*i) += 2;
      } else if ((**i >= 65 && **i <= 90) || (**i >= 97 && **i <= 122)) {
        unsigned char* str = malloc(1024);
        int j = 0;

        while ( ((**i >= 65 && **i <= 90) || (**i >= 97 && **i <= 122)) && j < 1024) {
          str[j++] = *((*i)++);
        }

        str[j] = 0;
        tok->sym = ident;
        tok->data = str;
      } else {
        tok = NULL;
      }
      break;
  }

  return tok;
}

void nuonParseError (nuonState_t* state)
{
  return;
}

int nuonAccept (nuonState_t* state)
{
  return 0;
}

int nuonExpect (nuonState_t* state)
{
  return 0;
}

int nuonPeek (nuonState_t* state)
{
  return 0;
}

void nuonParse (unsigned char** prog)
{
  nuonState_t* state = malloc(sizeof(nuonState_t));
  state->prog = prog;

  while ((state->tok = nuonNextToken(state->prog))) {
    printf("%d %s\n", state->tok->sym, state->tok->data);
    free(state->tok);
  }
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