#ifndef _NUON_H
#define _NUON_H

#define NUON_KEY_MAX 256

typedef unsigned long int nWord_t;
typedef unsigned char nByte_t;

void* nuonMalloc (nWord_t);
void nuonFree (void *);

int nuonStrlen (unsigned char *);
int nuonStrncmp (unsigned char *, unsigned char *);

#endif