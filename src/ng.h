#ifndef _NUON_H
#define _NUON_H

#define NUON_KEY_MAX 256

typedef unsigned long int word_t;

void* nuonMalloc (word_t);
void nuonFree (void *);

int nuonStrlen (unsigned char *);
int nuonStrncmp (unsigned char *, unsigned char *);

#endif