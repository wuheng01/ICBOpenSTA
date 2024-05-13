/*
Mini abc: extra utilities
*/

#include <stdlib.h>
#include <sstream>
#include <cstring>
#include <ExtraUtil.hh>
#include <abcglobal.hh>
#include <cassert>

void ExtraUtil::PrintHex(std::stringstream& ss, word32 compacted_tt [], word32 var_count){
  word32 nDigits, Digit;
  int k;
    // write the number into the file
    nDigits = (1 << var_count) / 4;
    for ( k = nDigits - 1; k >= 0; k-- )
    {
        Digit = ((compacted_tt[k/8] >> ((k%8) * 4)) & 15);
        if ( Digit < 10 )
	  ss << Digit;
        else{
	  char op_val = 'a' + Digit -10;
	  ss << op_val;
	}
    }
}

word64 ExtraUtil::Extra_Truth6MinimumRoundMany1( word64 t, int* pStore, char* pCanonPerm,
							    unsigned* pCanonPhase )
{
    word64 tMin0, tMin=t;
    char pCanonPerm1[16];
    unsigned uCanonPhase1;
    switch ((* pCanonPhase) >> 7)
    {
    case 0 :
        {
            return Extra_Truth6MinimumRoundMany_noEBFC( t, pStore, pCanonPerm, pCanonPhase);
        }
    case 1 :
        {            
            return Extra_Truth6MinimumRoundMany( t, pStore, pCanonPerm, pCanonPhase);
        }
    case 2 :
        { 
            uCanonPhase1 = *pCanonPhase;
            uCanonPhase1 ^= (1 << 6);
            memcpy(pCanonPerm1,pCanonPerm,sizeof(char)*16);
            tMin0 = Extra_Truth6MinimumRoundMany_noEBFC( t, pStore, pCanonPerm, pCanonPhase);
            tMin =  Extra_Truth6MinimumRoundMany_noEBFC( ~t, pStore, pCanonPerm1, &uCanonPhase1);
            if(tMin0 <=tMin)
                return tMin0;
            else
            {
                *pCanonPhase = uCanonPhase1;
                memcpy(pCanonPerm,pCanonPerm1,sizeof(char)*16);
                return tMin;
            }
        }
    case 3 :
        {
            uCanonPhase1 = *pCanonPhase;
            uCanonPhase1 ^= (1 << 6);
            memcpy(pCanonPerm1,pCanonPerm,sizeof(char)*16);
            tMin0 = Extra_Truth6MinimumRoundMany( t, pStore, pCanonPerm, pCanonPhase);
            tMin =  Extra_Truth6MinimumRoundMany( ~t, pStore, pCanonPerm1, &uCanonPhase1);
            if(tMin0 <=tMin)
                return tMin0;
            else
            {
                *pCanonPhase = uCanonPhase1;
                memcpy(pCanonPerm,pCanonPerm1,sizeof(char)*16);
                return tMin;
            }
        }
    }
    return Extra_Truth6MinimumRoundMany( t, pStore, pCanonPerm, pCanonPhase);
}



word64 ExtraUtil::Extra_Truth6MinimumRoundMany_noEBFC( word64 t, int* pStore,
								  char* pCanonPerm, unsigned* pCanonPhase )
{
    int i, bitInfoTemp;
    word64 tMin0, tMin=t;
    do
    {
        bitInfoTemp = pStore[0];
        tMin0 = tMin;
        for ( i = 0; i < 5; i++ )
        {
            if(bitInfoTemp == pStore[i+1])          
                tMin = Extra_Truth6MinimumRoundOne_noEBFC( tMin, i, pCanonPerm, pCanonPhase );         
            else
                bitInfoTemp = pStore[i+1];
        } 
    }while ( tMin0 != tMin );
    return tMin;
}

word64 ExtraUtil::Extra_Truth6MinimumRoundOne_noEBFC( word64 t, int iVar,  char* pCanonPerm, unsigned* pCanonPhase)
{
    word64 tMin;     
    assert( iVar >= 0 && iVar < 5 );  
    
    tMin = Extra_Truth6SwapAdjacent( t, iVar );   // b a
    if(t<tMin)
        return t;
    else
    {
        (* pCanonPhase) = adjustInfoAfterSwap(pCanonPerm, * pCanonPhase, iVar, 4);
        return tMin;
    }
}

//32 bit function
unsigned ExtraUtil::adjustInfoAfterSwap(char* pCanonPerm, unsigned uCanonPhase, int iVar, unsigned info)
{   
    if(info<4)
        return (uCanonPhase ^= (info << iVar));
    else
    {
        char temp;
        uCanonPhase ^= ((info-4) << iVar);
        temp=pCanonPerm[iVar];
        pCanonPerm[iVar]=pCanonPerm[iVar+1];
        pCanonPerm[iVar+1]=temp;
        if ( ((uCanonPhase & (1 << iVar)) > 0) != ((uCanonPhase & (1 << (iVar+1))) > 0) )
        {
            uCanonPhase ^= (1 << iVar);
            uCanonPhase ^= (1 << (iVar+1));
        }
        return uCanonPhase; 
    }
}



word64 ExtraUtil::Extra_Truth6SwapAdjacent( word64 t, int iVar )
{
    // variable swapping code
    static word64 PMasks[5][3] = {
        { ABCLITE_CONST_64(0x9999999999999999), ABCLITE_CONST_64(0x2222222222222222), ABCLITE_CONST_64(0x4444444444444444) },
        { ABCLITE_CONST_64(0xC3C3C3C3C3C3C3C3), ABCLITE_CONST_64(0x0C0C0C0C0C0C0C0C), ABCLITE_CONST_64(0x3030303030303030) },
        { ABCLITE_CONST_64(0xF00FF00FF00FF00F), ABCLITE_CONST_64(0x00F000F000F000F0), ABCLITE_CONST_64(0x0F000F000F000F00) },
        { ABCLITE_CONST_64(0xFF0000FFFF0000FF), ABCLITE_CONST_64(0x0000FF000000FF00), ABCLITE_CONST_64(0x00FF000000FF0000) },
        { ABCLITE_CONST_64(0xFFFF00000000FFFF), ABCLITE_CONST_64(0x00000000FFFF0000), ABCLITE_CONST_64(0x0000FFFF00000000) }
    };
    assert( iVar < 5 );
    return (t & PMasks[iVar][0]) | ((t & PMasks[iVar][1]) << (1 << iVar)) | ((t & PMasks[iVar][2]) >> (1 << iVar));
}


word64 ExtraUtil::Extra_Truth6MinimumRoundMany( word64 t, int* pStore,
							   char* pCanonPerm, unsigned* pCanonPhase )
{
  int i, bitInfoTemp;
    word64 tMin0, tMin=t;
    do
    {
        bitInfoTemp = pStore[0];
        tMin0 = tMin;
        for ( i = 0; i < 5; i++ )
        {
            if(bitInfoTemp == pStore[i+1])          
                tMin = Extra_Truth6MinimumRoundOne( tMin, i, pCanonPerm, pCanonPhase );         
            else
                bitInfoTemp = pStore[i+1];
        }
    }while ( tMin0 != tMin );
    return tMin;
}

word64 ExtraUtil::Extra_Truth6MinimumRoundOne( word64 t, int iVar, char* pCanonPerm, unsigned* pCanonPhase )
{
    word64 tCur, tMin = t; // ab 
    unsigned info =0;
    assert( iVar >= 0 && iVar < 5 );
    
    tCur = Extra_Truth6ChangePhase( t, iVar );    // !a b
    if(tCur<tMin)
    {
        info = 1;
        tMin = tCur;
    }
    tCur = Extra_Truth6ChangePhase( t, iVar+1 );  // a !b
    if(tCur<tMin)
    {
        info = 2;
        tMin = tCur;
    }
    tCur = Extra_Truth6ChangePhase( tCur, iVar ); // !a !b
    if(tCur<tMin)
    {
        info = 3;
        tMin = tCur;
    }
    
    t    = Extra_Truth6SwapAdjacent( t, iVar );   // b a
    if(t<tMin)
    {
        info = 4;
        tMin = t;
    }
    
    tCur = Extra_Truth6ChangePhase( t, iVar );    // !b a
    if(tCur<tMin)
    {
        info = 6;
        tMin = tCur;
    }
    tCur = Extra_Truth6ChangePhase( t, iVar+1 );  // b !a
    if(tCur<tMin)
    {
        info = 5;
        tMin = tCur;
    }
    tCur = Extra_Truth6ChangePhase( tCur, iVar ); // !b !a
    if(tCur<tMin)
    {
        (* pCanonPhase) = adjustInfoAfterSwap(pCanonPerm, * pCanonPhase, iVar, 7);
        return tCur;
    }
    else
    {
        (* pCanonPhase) = adjustInfoAfterSwap(pCanonPerm, * pCanonPhase, iVar, info);
        return tMin;
    }
}

word64 ExtraUtil::Extra_Truth6ChangePhase( word64 t, int iVar)
{
    // elementary truth tables
    static word64 Truth6[6] = {
        ABCLITE_CONST_64(0xAAAAAAAAAAAAAAAA),
        ABCLITE_CONST_64(0xCCCCCCCCCCCCCCCC),
        ABCLITE_CONST_64(0xF0F0F0F0F0F0F0F0),
        ABCLITE_CONST_64(0xFF00FF00FF00FF00),
        ABCLITE_CONST_64(0xFFFF0000FFFF0000),
        ABCLITE_CONST_64(0xFFFFFFFF00000000)
    };
    assert( iVar < 6 );
    return ((t & ~Truth6[iVar]) << (1 << iVar)) | ((t & Truth6[iVar]) >> (1 << iVar));
}

char*
ExtraUtil::Extra_UtilStrsav(const char* s){
     if(s == NULL) {  
       return NULL;
    }
    else {
       return strcpy(ABCLITE_ALLOC(char, strlen(s)+1), s);
    }
}

char *
ExtraUtil::Extra_FileNameGenericAppend( char * pBase, char * pSuffix )
{
    static char Buffer[1000];
    char * pDot;
    assert( strlen(pBase) + strlen(pSuffix) < 1000 );
    strcpy( Buffer, pBase );
    if ( (pDot = strrchr( Buffer, '.' )) )
        *pDot = 0;
    strcat( Buffer, pSuffix );
    return Buffer;
}
