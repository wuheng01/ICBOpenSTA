/*
Mini abc
*/

#include "ExtraUtil.hh"
#include "Npnencode.hh"
#include "Kit.hh"
#include <cassert>


void Npnencode::luckyCanonicizer_final_fast_16Vars(word64* pInOut, int  nVars, int nWords,
						   int * pStore, char * pCanonPerm,
						   unsigned* pCanonPhase)

{

    assert( nVars > 6 && nVars <= 16 );
    (* pCanonPhase) = Kit::Kit_TruthSemiCanonicize_Yasha1( pInOut, nVars, pCanonPerm, pStore );
    
    //  luckyCanonicizerS_F_first_16Vars1(pInOut, nVars, nWords, pStore, pCanonPerm, pCanonPhase ); 

}

word64 Npnencode::luckyCanonicizer_final_fast_6Vars(word64 InOut,
						    int* pStore,
						    char* pCanonPerm,
						    unsigned* pCanonPhase)
{
  (* pCanonPhase) = Kit::Kit_TruthSemiCanonicize_Yasha1( &InOut, 6, pCanonPerm, pStore);
  word64 ret= ExtraUtil::Extra_Truth6MinimumRoundMany1(InOut, pStore, pCanonPerm, pCanonPhase);
  return ret;
}


void Npnencode::resetPCanonPermArray_6Vars(char* x)
{
    x[0]='a';
    x[1]='b';
    x[2]='c';
    x[3]='d';
    x[4]='e';
    x[5]='f';
}

void Npnencode::resetPCanonPermArray(char* x, int nVars)
{
    int i;
    for(i=0;i<nVars;i++)
        x[i] = 'a'+i;
}

unsigned Npnencode::luckyCanonicizer_final_fast(word64* pInOut,
						int nVars,
						char* pCanonPerm){
  int nWords;
  int pStore[16];
  unsigned uCanonPhase=0;
  
  if (nVars <= 6){
    pInOut[0] = luckyCanonicizer_final_fast_6Vars( pInOut[0], pStore, pCanonPerm, &uCanonPhase);
    assert(pInOut[0] == pInOut[0]);
    return uCanonPhase;
  }
  else if (nVars <= 16){
    nWords = (nVars <= 6) ? 1 : (1 << (nVars - 6));
    luckyCanonicizer_final_fast_16Vars( pInOut, nVars, nWords, pStore, pCanonPerm, &uCanonPhase );
  }
  else assert(0);
  return uCanonPhase;
}
