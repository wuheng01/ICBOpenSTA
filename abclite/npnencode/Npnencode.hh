#pragma once

#include <abcglobal.hh>

class Npnencode {
public:

  
  using TtStore_t = struct {
    int nVars;
    int nWords;
    int nFuncs;
    word64** pFuncs;
  };

  using varInfo = struct {
    int direction;
    int position;
  };
    
  using swapInfo = struct {
    varInfo* posArray;
    int* realArray;
    int  varN;
    int positionToSwap1;
    int positionToSwap2;
  };

// allocate and clear memory to store 'nTruths' truth tables of 'nVars' variables
  static inline TtStore_t * TruthStoreAlloc( int nVars, int nFuncs )
  {
    TtStore_t * p;
    int i;
    p = (TtStore_t *)malloc( sizeof(TtStore_t) );
    p->nVars  =  nVars;
    p->nWords = (nVars < 7) ? 1 : (1 << (nVars-6));
    p->nFuncs =  nFuncs;
    // alloc array of 'nFuncs' pointers to truth tables
    p->pFuncs = (word64 **)malloc( sizeof(word64 *) * p->nFuncs );
    // alloc storage for 'nFuncs' truth tables as one chunk of memory
    p->pFuncs[0] = (word64 *)calloc( sizeof(word64), p->nFuncs * p->nWords );
    // split it up into individual truth tables
    for ( i = 1; i < p->nFuncs; i++ )
        p->pFuncs[i] = p->pFuncs[i-1] + p->nWords;
    return p;
}
  
// free memory previously allocated for storing truth tables
  static inline void TruthStoreFree( TtStore_t * p )
{
    free( p->pFuncs[0] );
    free( p->pFuncs );
    free( p );
}

  static void resetPCanonPermArray_6Vars(char* x);
  static void resetPCanonPermArray(char* x, int nVars);  
  static void luckyCanonicizer_final_fast_16Vars(word64* pInOut,
						int  nVars, int nWords,
						int * pStore,
						char * pCanonPerm, unsigned* pCanonPhase);

 static   word64 luckyCanonicizer_final_fast_6Vars(word64 InOut,
						 int* pStore,
						 char* pCanonPerm,
						 unsigned* pCanonPhase);
  
 static   unsigned luckyCanonicizer_final_fast(word64*  pinoout,
					       int nVars,
					       char* pCanonPerm);

  
  

};


  
