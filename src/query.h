/* query.h
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

#ifndef _QUERY_H
#define _QUERY_H

typedef struct nuonToken nuonToken;
typedef enum nuonSymbol  nuonSymbol;
typedef struct nuonState nuonState;

enum nuonSymbol {
  create_sym, return_sym, node_sym, 
  set_sym,    where_sym,  and_sym, 
  select_sym, period,     comma,
  qmark,      equals,     ident,
  string,     arrow
};

struct nuonToken {
  nuonSymbol      sym;
  unsigned char* data;
};

struct nuonState {
  nuonToken* tok;
  unsigned char** prog;
};

#define NUON_CMP(x, y, z) strncmp((const char *)x, (const char *)y, z)

#define NUON_IS_CREATE_SYM(x) !NUON_CMP(x, "CREATE", 6) || !NUON_CMP(x, "create", 6)
#define NUON_IS_SELECT_SYM(x) !NUON_CMP(x, "SELECT", 6) || !NUON_CMP(x, "select", 6)
#define NUON_IS_RETURN_SYM(x) !NUON_CMP(x, "RETURN", 6) || !NUON_CMP(x, "return", 6)
#define NUON_IS_NODE_SYM(x)   !NUON_CMP(x, "NODE", 4)   || !NUON_CMP(x, "node", 4)
#define NUON_IS_SET_SYM(x)    !NUON_CMP(x, "SET", 3)    || !NUON_CMP(x, "set", 3)
#define NUON_IS_WHERE_SYM(x)  !NUON_CMP(x, "WHERE", 5)  || !NUON_CMP(x, "where", 5)
#define NUON_IS_AND_SYM(x)    !NUON_CMP(x, "AND", 3)    || !NUON_CMP(x, "and", 3)
#define NUON_IS_ARROW_SYM(x)  !NUON_CMP(x, "->", 2) 

unsigned char*  nuonReadLine      (FILE*);
nuonToken*      nuonNextToken     (unsigned char**);
void            nuonParse         (unsigned char**);
int             nuonAccept        (nuonState*, nuonSymbol);
int             nuonExpect        (nuonState*, nuonSymbol);
int             nuonPeek          (nuonState*, nuonSymbol);
void            nuonGetSym        (nuonState*);

#endif
