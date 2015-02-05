/* token.c
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

#define IS_CREATE_TOK(x) !strncmp((const char*)x, "CREATE", 6) || !strncmp((const char*)x, "create", 6)
#define IS_MATCH_TOK(x) !strncmp((const char*)x, "MATCH", 5) || !strncmp((const char*)x, "match", 5)

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
    default:
      if ( **i == '"' ) {
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
      } else if ( IS_CREATE_TOK(*i) ) {
        token->sym = create;
        (*i) += 6;
      } else if ( IS_MATCH_TOK(*i) ) {
        token->sym = create;
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