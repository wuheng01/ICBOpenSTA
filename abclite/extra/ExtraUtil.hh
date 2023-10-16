/*
Abc-lite: Abc Objects
*/

#pragma once

#include <abcglobal.hh>

#include <sstream>
class ExtraUtil {

public:

  //32 bit word utilities
  static void PrintHex(std::stringstream& op, word32[], word32 var_count);

  //64 bit word utilities (used for npn).
  static word64 Extra_Truth6MinimumRoundMany1( word64 t, int* pStore, char* pCanonPerm,
					     unsigned* pCanonPhase );
  static word64 Extra_Truth6MinimumRoundMany( word64 t, int* pStore, char* pCanonPerm,
					    unsigned* pCanonPhase )  ;
  static word64 Extra_Truth6MinimumRoundOne( word64 t, int iVar, char* pCanonPerm, unsigned* pCanonPhase );
  static word64 Extra_Truth6SwapAdjacent( word64 t, int iVar );
  static unsigned adjustInfoAfterSwap(char* pCanonPerm, unsigned uCanonPhase, int iVar, unsigned info);
  static word64 Extra_Truth6MinimumRoundOne_noEBFC( word64 t, int iVar,  char* pCanonPerm, unsigned* pCanonPhase);
  static word64 Extra_Truth6MinimumRoundMany_noEBFC( word64 t, int* pStore, char* pCanonPerm, unsigned* pCanonPhase );
  static word64 Extra_Truth6ChangePhase(word64 t, int iVar);

  static char* Extra_UtilStrsav(const char* s);
  static char* Extra_FileNameGenericAppend( char * pBase, char * pSuffix );
};
