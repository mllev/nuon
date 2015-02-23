/* nuon
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

#include "linenoise.h"
#include "nuon.h"

void teststr(char * str);
void query_parserTest (void);
char* readline (FILE*);

char* readline (FILE* f)
{
  unsigned char* buf = NULL;
  int c = 0, i = 0, bufsize = 10;

  buf = malloc(bufsize + 1);
  memset(buf, 0, bufsize + 1);

  while ( (c = fgetc(f)) != EOF ) {
    if ( i == bufsize ) {
      buf = realloc(buf, (bufsize += 10) + 1);
    }
    buf[i++] = (unsigned char)c;
    if ( buf[i - 1] == '\n' ) {
      break;
    }
  }

  buf[i] = 0;

  return (char *)buf;
}


void query_parserTest (void)
{
  Graph* g = graph_init(100000);
  char* line;
  while((line = linenoise("nuon> ")) != NULL) {
    parse(g, (unsigned char *)line);
    linenoiseHistoryAdd(line);
    free(line);
  }
  // while ( 1 ) {
  //   printf("nuon> ");
  //   line = readline(stdin);
  //   parse(g, (unsigned char *)line);
  //   free(line);
  //   line = NULL;
  // }
}

void teststr(char * str)
{
  static int i = 0;
  sprintf(str, "%d", ++i);
}

int main (void)
{
  query_parserTest();
  return 0;
}