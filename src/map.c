/* map.c
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
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "map.h"

int height ();
map_node_t* map_node_init (int, const char*, void*);
void map_node_destroy (map_node_t *);

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

static inline int strncmpsafe (const char* k0, unsigned char* k1)
{
  int kl0, kl1, kl;

  kl0 = strlen(k0);
  kl1 = strlen((const char*)k1);
  kl = (kl0 > kl1 ? kl0 : kl1);
  
  return strncmp((const char*)k0, (const char*)k1, kl);
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
