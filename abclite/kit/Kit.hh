/*
Mini abc: kit package.
*/
#pragma once

#include <abcglobal.hh> //definition of word size
#include <cstring> //memset

static word64 mask0[6] = { ABCLITE_CONST_64(0x5555555555555555),ABCLITE_CONST_64(0x3333333333333333), ABCLITE_CONST_64(0x0F0F0F0F0F0F0F0F),ABCLITE_CONST_64(0x00FF00FF00FF00FF),ABCLITE_CONST_64(0x0000FFFF0000FFFF), ABCLITE_CONST_64(0x00000000FFFFFFFF)};
class Kit {
public:

static inline void Kit_TruthOr( unsigned * pOut, unsigned * pIn0, unsigned * pIn1, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        pOut[w] = pIn0[w] | pIn1[w];
}
static inline void Kit_TruthXor( unsigned * pOut, unsigned * pIn0, unsigned * pIn1, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        pOut[w] = pIn0[w] ^ pIn1[w];
}

static inline void Kit_TruthAnd( unsigned * pOut, unsigned * pIn0, unsigned * pIn1, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        pOut[w] = pIn0[w] & pIn1[w];
}  
static inline void Kit_TruthIthVar( unsigned * pTruth, int nVars, int iVar )
{
    unsigned Masks[5] = { 0xAAAAAAAA, 0xCCCCCCCC, 0xF0F0F0F0, 0xFF00FF00, 0xFFFF0000 };
    int k, nWords = (nVars <= 5 ? 1 : (1 << (nVars - 5)));
    if ( iVar < 5 )
    {
        for ( k = 0; k < nWords; k++ )
            pTruth[k] = Masks[iVar];
    }
    else
    {
        for ( k = 0; k < nWords; k++ )
            if ( k & (1 << (iVar-5)) )
                pTruth[k] = ~(unsigned)0;
            else
                pTruth[k] = 0;
    }
}

  static inline void Kit_TruthFill( unsigned * pOut, int nVars )
  {
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        pOut[w] = ~(unsigned)0;
  }
  static void Kit_TruthSwapAdjacentVars( word32 * pOut, unsigned * pIn, int nVars, int iVar );
  static void Kit_TruthChangePhase( word32 * pTruth, int nVars, int iVar );
  static inline void Kit_TruthCopy( word32 * pOut, word32 * pIn, int nVars )
  {
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        pOut[w] = pIn[w];
  }
  
  static inline int Kit_WordCountOnes( word32 uWord )
  {
    uWord = (uWord & 0x55555555) + ((uWord>>1) & 0x55555555);
    uWord = (uWord & 0x33333333) + ((uWord>>2) & 0x33333333);
    uWord = (uWord & 0x0F0F0F0F) + ((uWord>>4) & 0x0F0F0F0F);
    uWord = (uWord & 0x00FF00FF) + ((uWord>>8) & 0x00FF00FF);
    return  (uWord & 0x0000FFFF) + (uWord>>16);
  }
  
  static inline int Kit_TruthCountOnes( word32 * pIn, int nVars )
  {
    int w, Counter = 0;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        Counter += Kit_WordCountOnes(pIn[w]);
    return Counter;
  }

  static inline void Kit_TruthNot( word32 * pOut, word32 * pIn, int nVars )
  {
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        pOut[w] = ~pIn[w];
  }
  
  
  static inline int   Kit_TruthWordNum( int nVars )  { return nVars <= 5 ? 1 : (1 << (nVars - 5));}

  
  static inline int   Kit_TruthWordNum( word32* p, int nVars )  {(void)p; return nVars <= 5 ? 1 : (1 << (nVars - 5));}
  static inline int   Kit_TruthWordNum( word64* p, int nVars )  {(void)p; return nVars <= 6 ? 1 : (1 << (nVars - 6));}
  
  static void         Kit_TruthCountOnesInCofs( word32 * pTruth, int nVars, int * pStore );
  
  static unsigned Kit_TruthSemiCanonicize( word32 * pInOut, word32 * pAux, int nVars, char * pCanonPerm );
  static unsigned Kit_TruthSemiCanonicize_Yasha1(word64* pInOut, int nVars, char * pCanonPerm, int * pStore );


  

  static bool Kit_IsBitSet(/*word32*/unsigned* p, int var_count, int j);
  static bool Kit_IsBitSet(word64* p, int var_count, int j);  
  
  //64 bit word handling
  static inline int Kit_TruthWordNum_64bit( int nVars )  { return nVars <= 6 ? 1 : (1 << (nVars - 6));}
  
  static inline int Kit_WordCountOnes_64bit(word64 x)
{
    x = x - ((x >> 1) & ABCLITE_CONST_64(0x5555555555555555));   
    x = (x & ABCLITE_CONST_64(0x3333333333333333)) + ((x >> 2) & ABCLITE_CONST_64(0x3333333333333333));    
    x = (x + (x >> 4)) & ABCLITE_CONST_64(0x0F0F0F0F0F0F0F0F);    
    x = x + (x >> 8);
    x = x + (x >> 16);
    x = x + (x >> 32); 
    return (int)(x & 0xFF);
}

  static inline int Kit_TruthCountOnes_64bit( word64* pIn, int nVars )
  {
    int w, Counter = 0;
    for ( w = Kit_TruthWordNum_64bit(nVars)-1; w >= 0; w-- )
        Counter += Kit_WordCountOnes_64bit(pIn[w]);
    return Counter;
}

  static inline void Kit_TruthCountOnesInCofs_64bit( word64 * pTruth, int nVars, int * pStore )
  {    
    int nWords = Kit_TruthWordNum_64bit( nVars );
    int i, k, Counter;
    memset( pStore, 0, sizeof(int) * nVars );
    if ( nVars <= 6 )
    {
        if ( nVars > 0 )        
            pStore[0] = Kit_WordCountOnes_64bit( pTruth[0] & ABCLITE_CONST_64(0x5555555555555555) );  
        if ( nVars > 1 )       
            pStore[1] = Kit_WordCountOnes_64bit( pTruth[0] & ABCLITE_CONST_64(0x3333333333333333) );     
        if ( nVars > 2 )       
            pStore[2] = Kit_WordCountOnes_64bit( pTruth[0] & ABCLITE_CONST_64(0x0F0F0F0F0F0F0F0F) );   
        if ( nVars > 3 )       
            pStore[3] = Kit_WordCountOnes_64bit( pTruth[0] & ABCLITE_CONST_64(0x00FF00FF00FF00FF) );     
        if ( nVars > 4 )       
            pStore[4] = Kit_WordCountOnes_64bit( pTruth[0] & ABCLITE_CONST_64(0x0000FFFF0000FFFF) ); 
        if ( nVars > 5 )       
            pStore[5] = Kit_WordCountOnes_64bit( pTruth[0] & ABCLITE_CONST_64(0x00000000FFFFFFFF) );      
        return;
    }
    // nVars > 6
    // count 1's for all other variables
    for ( k = 0; k < nWords; k++ )
    {
        Counter = Kit_WordCountOnes_64bit( pTruth[k] );
        for ( i = 6; i < nVars; i++ )
            if ( (k & (1 << (i-6))) == 0)
                pStore[i] += Counter;
    }
    // count 1's for the first six variables
    for ( k = nWords/2; k>0; k-- )
    {
        pStore[0] += Kit_WordCountOnes_64bit( (pTruth[0] & ABCLITE_CONST_64(0x5555555555555555)) | ((pTruth[1] & ABCLITE_CONST_64(0x5555555555555555)) <<  1) );
        pStore[1] += Kit_WordCountOnes_64bit( (pTruth[0] & ABCLITE_CONST_64(0x3333333333333333)) | ((pTruth[1] & ABCLITE_CONST_64(0x3333333333333333)) <<  2) );
        pStore[2] += Kit_WordCountOnes_64bit( (pTruth[0] & ABCLITE_CONST_64(0x0F0F0F0F0F0F0F0F)) | ((pTruth[1] & ABCLITE_CONST_64(0x0F0F0F0F0F0F0F0F)) <<  4) );
        pStore[3] += Kit_WordCountOnes_64bit( (pTruth[0] & ABCLITE_CONST_64(0x00FF00FF00FF00FF)) | ((pTruth[1] & ABCLITE_CONST_64(0x00FF00FF00FF00FF)) <<  8) );
        pStore[4] += Kit_WordCountOnes_64bit( (pTruth[0] & ABCLITE_CONST_64(0x0000FFFF0000FFFF)) | ((pTruth[1] & ABCLITE_CONST_64(0x0000FFFF0000FFFF)) <<  16) );
        pStore[5] += Kit_WordCountOnes_64bit( (pTruth[0] & ABCLITE_CONST_64(0x00000000FFFFFFFF)) | ((pTruth[1] & ABCLITE_CONST_64(0x00000000FFFFFFFF)) <<  32) );  
        pTruth += 2;
    }
  }
  static void Kit_TruthSwapAdjacentVars_64bit( word64 * pInOut, int nVars, int iVar );
  static void Kit_TruthCopy_64bit( word64 * pOut, word64 * pIn, int nVars );
  static void Kit_TruthNot_64bit(word64 * pIn, int nVars );
  static void Kit_TruthChangePhase_64bit( word64 * pInOut, int nVars, int iVar );

  
  static bool Kit_IsBuffer(word64* p, int nVars);
  static bool Kit_IsInverter(word64* p, int nVars);
  static bool Kit_IsBuffer(word32* p, int nVars);
  static bool Kit_IsInverter(word32* p, int nVars);

};
