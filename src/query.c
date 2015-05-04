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

Token* nuonNextToken (unsigned char** i)
{
  Token* token = malloc(sizeof(Token));
  token->data = NULL;

  while (**i && (**i == ' ' || **i == '\n' || **i == '\t')) {
    (*i)++;
  }

  switch (**i) {
    case '.':
      (*i)++;
      token->sym = period;
      break;
    case '=':
      (*i)++;
      token->sym = equals;
      break;
    case ',':
      (*i)++;
      token->sym = comma;
      break;
    case '?':
      (*i)++;
      token->sym = qmark;
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
        token->sym = string;
        token->data = str;
      } while (0);
      break;
    default:
      if (NUON_IS_CREATE_TOK(*i)) {
        token->sym = create_sym;
        (*i) += 6;
      } else if (NUON_IS_SET_TOK(*i)) {
        token->sym = set_sym;
        (*i) += 3;
      } else if (NUON_IS_SELECT_TOK(*i)) {
        token->sym = select_sym;
        (*i) += 6;
      } else if (NUON_IS_RETURN_TOK(*i)) {
        token->sym = return_sym;
        (*i) += 6;
      } else if (NUON_IS_WHERE_TOK(*i)) {
        token->sym = where_sym;
        (*i) += 5;
      } else if (NUON_IS_AND_TOK(*i)) {
        token->sym = and_sym;
        (*i) += 3;
      } else if (NUON_IS_NODE_TOK(*i)) {
        token->sym = node_sym;
        (*i) += 4;
      } else if (NUON_IS_ARROW_TOK(*i)) {
        token->sym = arrow;
        (*i) += 2;
      } else if ((**i >= 65 && **i <= 90) || (**i >= 97 && **i <= 122)) {
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

unsigned char* nuonReadLine (FILE* f) {
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

int main (void) {
  unsigned char *line, *l;
  Token* t;

  while (1) {
    printf("nuon> ");
    line = nuonReadLine((FILE* )stdin);
    l = line;
    while ((t = nuonNextToken(&l))) {
      printf("%d %s\n", t->sym, t->data);
      free(t);
    }
    free(line);
  }
}