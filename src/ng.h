#ifndef _NUON_H
#define _NUON_H

#define NUON_KEY_MAX 256

typedef unsigned long int nWord_t;
typedef unsigned char nByte_t;

void* nuonMalloc (nWord_t);
void nuonFree (void *);

int nuonStrlen (unsigned char *);
int nuonStrncmp (unsigned char *, unsigned char *);

#define NUON_TRIE_LIMIT 128 /* 7 bit ascii */

typedef union trieElem TrieElem;
typedef TrieElem Trie;

const int nuonTrieVal = NUON_TRIE_LIMIT;
const int nuonTrieSubCount = nuonTrieVal + 1;

union trieElem {
  nWord_t sub[NUON_TRIE_LIMIT + 2];
};

/* map api */
Trie* nuonTrieInit (void);
int nuonTrieAdd (Trie* t, nByte_t *, void *);
void* nuonTrieGet (Trie* t, nByte_t* key);

#endif