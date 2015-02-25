#ifndef _NUON_H
#define _NUON_H

#define NUON_KEY_MAX 32

typedef unsigned long int nWord_t;
typedef unsigned char nByte_t;

void* nuonMalloc (nWord_t);
void nuonFree (void *);

int nuonStrlen (unsigned char *);
int nuonStrncmp (unsigned char *, unsigned char *);

/* to change trie span, both variables below must be changed */
#define NUON_TRIE_SPAN  3 /* trie span */
#define NUON_TRIE_LIMIT 8 /* 2 ^ span */

#define NUON_TRIE_POOL  2048
#define NUON_BYTE_SIZE  8

typedef union trieElem TrieElem;
typedef TrieElem Trie;

const int trieVal = NUON_TRIE_LIMIT;
const int trieMask = 0xff >> (NUON_BYTE_SIZE - NUON_TRIE_SPAN);
const int trieOff = NUON_BYTE_SIZE / NUON_TRIE_SPAN;
const int trieDiff = NUON_BYTE_SIZE - NUON_TRIE_SPAN;

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