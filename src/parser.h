/* parser.h
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
 
#ifndef _PARSER_H
#define _PARSER_H

#include "graph.h"

/*** Grammar ***

KeyValueList ::= 
    ident ":" string
  | ident ":" string, KeyValueList

Data ::= 
  "{" KeyValueList "}"

Type ::=
    ident "as" ident 
  | ident

Node ::=
    "(" Type Data ")"

Edge ::=
    "-[" ident "]->"

NodeList ::= 
    Node 
  | Node Edge Node
  | Node "," NodeList
  | Node Edge Node "," NodeList

MatchNodeList ::= 
    Node 
  | Node "," NodeList

Create ::=
    "create" NodeList

Match ::= 
    "match" MatchNodeList SetList Return

Property ::=
    ident "." ident

IdentList ::=
    ident
  | ident "." ident
  | ident "," IdentList
  | ident "." ident "," IdentList

Return ::=
    "return" IdentList
  | null

SetList ::=
    "set" Set SetList
  | null

Set ::=
    Property "=" string
  | Node Edge Node

Expr ::=
    Match Expr
  | Create Expr
  | null

*** Examples *** HEAVILY inspired by Neo4j's Cypher query language ***

CREATE (p:Person {"name": "Matt"}), (q:Person {"name": "Jordan"}), (p) -[k:knows]-> (q)

MATCH (p:Person {"name": "Matt"}) -[:knows]-> (q) SET p.name = "jim" SET q.age = "25" SET (p) -[:hates]-> (q) return p, q

***** opcodes *****

***************/

void parse (Graph*, unsigned char*);

#endif