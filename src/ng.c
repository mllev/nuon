/* NUON - Graph Database
 *
 *        / /|\ \ 
 *     / / /|||\ \ \
 *    | | |     | | |
 *    | | |     | | |
 *    \ \ \     / / /
 *     \ \ \   / / /
 *      \ \ \ / / /
 *       \ \ \ / /
 *        \ \ \ /  For my beautiful sister Holly and her triumphant battle against cancer. 
 *        /\ \ \   Your courage is truly humbling.
 *       / /\ \ \
 *      / / /\ \ \
 *     / / /  \ \ \
 *    / / /    \ \ \
 *   / / /      \ \ \
 *    / /        \ \ \
 *                \ \
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

#include "ng.h"

void* nuonMalloc (word_t size) {
  void* buf = malloc(size);
  if (!buf) {
    fprintf(stderr, "Fatal: out of memory.");
    abort();
  }
  return buf;
}

void nuonFree (void* buf) {
  free(buf);
}

int nuonStrlen (unsigned char * str) {
  unsigned int len, nlen;
  len = (unsigned int)strlen((const char *)str);
  nlen = (len > NUON_KEY_MAX ? NUON_KEY_MAX : len);
  return (int)nlen;
}

int nuonStrncmp (unsigned char* k0, unsigned char* k1) {
  int kl0, kl1, kl, res;
  kl0 = nuonStrlen(k0);
  kl1 = nuonStrlen(k1);
  kl = (kl0 < kl1 ? kl0 : kl1);
  res = strncmp((const char*)k0, (const char*)k1, kl);
  return (res ? res : kl0 == kl1 ? 0 : kl0 < kl1 ? -1 : 1);
}
