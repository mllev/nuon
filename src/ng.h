#ifndef _NUON_H
#define _NUON_H

#define NUON_KEY_MAX 32

typedef unsigned long int nWord_t;
typedef unsigned char nByte_t;

void* nuonMalloc (nWord_t);
void nuonFree (void *);

int nuonStrlen (unsigned char *);
int nuonStrncmp (unsigned char *, unsigned char *);

typedef unsigned long int nWord_t;
typedef unsigned char nByte_t;

#define NUON_TRIE_LIMIT 4 /* 2 ^ S where S is the span (2) */
#define NUON_TRIE_POOL 1024

typedef union trieElem TrieElem;
typedef TrieElem Trie;

const int nuonTrieVal = NUON_TRIE_LIMIT;

int bytes;

union trieElem {
  nWord_t sub[NUON_TRIE_LIMIT + 1];
};

Trie* nuonTrieInit (void);
int nuonTrieAdd (Trie* t, nByte_t *, void *);
void* nuonTrieGet (Trie* t, nByte_t* key);
Trie* nuonTrieElemInit (void);
TrieElem* nuonTrieFind (Trie *, nByte_t *, int);

#endif