#include <stdio.h>
#include <stdlib.h>
#include "mem.h"
#include "hash.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint64_t ZHASH(uint64_t z){
  z = (~z) + (z << 21);
  z = z    ^ (z >> 24);
  z = (z   + (z << 3)) + (z << 8);
  z = z    ^ (z >> 14);
  z = (z   + (z << 2)) + (z << 4);
  z = z    ^ (z >> 28);
  z = z    + (z << 31);
  return z;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

HASH *CreateHashTable(uint32_t nSym){ 
  HASH *H         = (HASH *) Calloc(1, sizeof(HASH));
  H->size         = HASH_SIZE;
  H->nSym         = nSym;
  H->entries      = (ENTRY  **) Calloc(H->size, sizeof(ENTRY *));
  H->entrySize    = (ENTMAX  *) Calloc(H->size, sizeof(ENTMAX));
  return H;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void InsertKey(HASH *H, uint32_t hi, uint64_t key, uint32_t s){
  H->entries[hi] = (ENTRY *) Realloc(H->entries[hi], 
                   (H->entrySize[hi] + 1) * sizeof(ENTRY), sizeof(ENTRY));
  H->entries[hi][H->entrySize[hi]].key = key;
  uint64_t i = 1;
  H->entries[hi][H->entrySize[hi]].counters = (i<<(s<<1));
  H->entrySize[hi]++;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

HCC GetHCCounters(HASH *H, uint64_t key){

 key = ZHASH(key);
 uint64_t x, hi = key % H->size;              //The hash index

 for(x = 0 ; x < H->entrySize[hi] ; ++x)
   if(H->entries[hi][x].key == key)     // If key found
     return H->entries[hi][x].counters;

  return 0;
  } 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void UpdateHashCounter(HASH *H, uint32_t sym, uint64_t im){
  uint32_t x;
  uint64_t idx = ZHASH(im);
  uint64_t s, hIndex = idx % H->size, counter, sc;

  for(x = 0 ; x < H->entrySize[hIndex] ; ++x) {
    if(H->entries[hIndex][x].key == idx) {

      sc = ((H->entries[hIndex][x].counters)>>(sym<<1)) & 0x03;
      if(sc == MAX_HASH_COUNTER) {
        for(s = 0 ; s < H->nSym ; ++s) {
	  counter = ((H->entries[hIndex][x].counters>>(s<<1))&0x03)>>1;
	  H->entries[hIndex][x].counters &= ~(0x03<<(s<<1));
	  H->entries[hIndex][x].counters |= (counter<<(s<<1));
	}
      }
      sc = (H->entries[hIndex][x].counters>>(sym<<1))&0x03;
      ++sc;
      H->entries[hIndex][x].counters &= ~(0x03<<(sym<<1));
      H->entries[hIndex][x].counters |= (sc<<(sym<<1));
      
      return;
    }
  }
  
  InsertKey(H, hIndex, idx, sym);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void RemoveHashTable(HASH *H){
  uint64_t x;
  for(x = 0 ; x < H->size ; ++x){
    if(H->entrySize[x] != 0){
      Free(H->entries[x]);
      }
    }
  Free(H->entries);
  Free(H->entrySize);
  Free(H);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

