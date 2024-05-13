#include <cassert>
#include <abcglobal.hh>
#include <Kit.hh>
#include <stdio.h>

void Kit::Kit_TruthChangePhase( word32 * pTruth, int nVars, int iVar )
{
    int nWords = Kit_TruthWordNum( nVars );
    int i, k, Step;
    unsigned Temp;

    assert( iVar < nVars );
    switch ( iVar )
    {
    case 0:
        for ( i = 0; i < nWords; i++ )
            pTruth[i] = ((pTruth[i] & 0x55555555) << 1) | ((pTruth[i] & 0xAAAAAAAA) >> 1);
        return;
    case 1:
        for ( i = 0; i < nWords; i++ )
            pTruth[i] = ((pTruth[i] & 0x33333333) << 2) | ((pTruth[i] & 0xCCCCCCCC) >> 2);
        return;
    case 2:
        for ( i = 0; i < nWords; i++ )
            pTruth[i] = ((pTruth[i] & 0x0F0F0F0F) << 4) | ((pTruth[i] & 0xF0F0F0F0) >> 4);
        return;
    case 3:
        for ( i = 0; i < nWords; i++ )
            pTruth[i] = ((pTruth[i] & 0x00FF00FF) << 8) | ((pTruth[i] & 0xFF00FF00) >> 8);
        return;
    case 4:
        for ( i = 0; i < nWords; i++ )
            pTruth[i] = ((pTruth[i] & 0x0000FFFF) << 16) | ((pTruth[i] & 0xFFFF0000) >> 16);
        return;
    default:
        Step = (1 << (iVar - 5));
        for ( k = 0; k < nWords; k += 2*Step )
        {
            for ( i = 0; i < Step; i++ )
            {
                Temp = pTruth[i];
                pTruth[i] = pTruth[Step+i];
                pTruth[Step+i] = Temp;
            }
            pTruth += 2*Step;
        }
        return;
    }
}


bool
Kit::Kit_IsBitSet(unsigned* p, int var_count, int ix){

  int n_words = Kit_TruthWordNum(p,var_count);
  int offset = ix % 32;
  int word_address = ix / 32;
  if (p[word_address] & (0x1 << offset))
    return true;
  return false;
}

bool
Kit::Kit_IsBitSet(word64* p, int var_count, int ix){
  int n_words = Kit_TruthWordNum_64bit(var_count);
  int offset = ix % 64;
  int word_address = ix / 64;
  if (p[word_address] & (0x1 << offset))
    return true;
  return false;
}





bool Kit::Kit_IsBuffer(word32* p, int n_vars){
  bool ret = true;
  int n_words = Kit_TruthWordNum(p,n_vars);
  assert(n_words > 0);
  for (int i=0; i < n_words; i++){
    if (p[i] != 0xAAAAAAAA)
      return false;
  }
  return ret;
}

bool Kit::Kit_IsBuffer(word64* p, int n_vars){
  bool ret = true;
  int n_words = Kit_TruthWordNum(p,n_vars);
  assert(n_words > 0);
  for (int i=0; i < n_words; i++){
    if (p[i] != 0xAAAAAAAAAAAAAAAA)
      return false;
  }
  return ret;
}

bool Kit::Kit_IsInverter(word32* p, int n_vars){
    bool ret = true;
    int n_words = Kit_TruthWordNum(p,n_vars);
    assert(n_words > 0);
    for (int i=0; i < n_words; i++){
      if (p[i] != 0x55555555)
	return false;
    }
    return ret;
}

bool Kit::Kit_IsInverter(word64* p, int n_vars){
    bool ret = true;
    int n_words = Kit_TruthWordNum(p,n_vars);
    assert(n_words > 0);
    for (int i=0; i < n_words; i++){
      if (p[i] != 0x5555555555555555)
	return false;
    }
    return ret;
}



void Kit::Kit_TruthSwapAdjacentVars( word32 * pOut, unsigned * pIn, int nVars, int iVar )
{
    static word32 PMasks[4][3] = {
        { 0x99999999, 0x22222222, 0x44444444 },
        { 0xC3C3C3C3, 0x0C0C0C0C, 0x30303030 },
        { 0xF00FF00F, 0x00F000F0, 0x0F000F00 },
        { 0xFF0000FF, 0x0000FF00, 0x00FF0000 }
    };
    int nWords = Kit_TruthWordNum( nVars );
    int i, k, Step, Shift;

    assert( iVar < nVars - 1 );
    if ( iVar < 4 )
    {
        Shift = (1 << iVar);
        for ( i = 0; i < nWords; i++ )
            pOut[i] = (pIn[i] & PMasks[iVar][0]) | ((pIn[i] & PMasks[iVar][1]) << Shift) | ((pIn[i] & PMasks[iVar][2]) >> Shift);
    }
    else if ( iVar > 4 )
    {
        Step = (1 << (iVar - 5));
        for ( k = 0; k < nWords; k += 4*Step )
        {
            for ( i = 0; i < Step; i++ )
                pOut[i] = pIn[i];
            for ( i = 0; i < Step; i++ )
                pOut[Step+i] = pIn[2*Step+i];
            for ( i = 0; i < Step; i++ )
                pOut[2*Step+i] = pIn[Step+i];
            for ( i = 0; i < Step; i++ )
                pOut[3*Step+i] = pIn[3*Step+i];
            pIn  += 4*Step;
            pOut += 4*Step;
        }
    }
    else // if ( iVar == 4 )
    {
        for ( i = 0; i < nWords; i += 2 )
        {
            pOut[i]   = (pIn[i]   & 0x0000FFFF) | ((pIn[i+1] & 0x0000FFFF) << 16);
            pOut[i+1] = (pIn[i+1] & 0xFFFF0000) | ((pIn[i]   & 0xFFFF0000) >> 16);
        }
    }
}

unsigned Kit::Kit_TruthSemiCanonicize( word32 * pInOut, word32 * pAux, int nVars, char * pCanonPerm )
{
    int pStore[32];
    word32 * pIn = pInOut, * pOut = pAux, * pTemp;
    int nWords = Kit_TruthWordNum( nVars );
    int i, Temp, fChange, Counter, nOnes;//, k, j, w, Limit;
    word32 uCanonPhase;

    // canonicize output
    uCanonPhase = 0;
    for ( i = 0; i < nVars; i++ )
        pCanonPerm[i] = i;

    nOnes = Kit_TruthCountOnes(pIn, nVars);
    //if(pIn[0] & 1)
    if ( (nOnes > nWords * 16) )//|| ((nOnes == nWords * 16) && (pIn[0] & 1)) )
    {
        uCanonPhase |= (1 << nVars);
        Kit_TruthNot( pIn, pIn, nVars );
    }

    // collect the minterm counts
    Kit_TruthCountOnesInCofs( pIn, nVars, pStore );
/*
    Kit_TruthCountOnesInCofsSlow( pIn, nVars, pStore2, pAux );
    for ( i = 0; i < 2*nVars; i++ )
    {
        assert( pStore[i] == pStore2[i] );
    }
*/
    // canonicize phase
    for ( i = 0; i < nVars; i++ )
    {
        if ( pStore[2*i+0] >= pStore[2*i+1] )
            continue;
        uCanonPhase |= (1 << i);
        Temp = pStore[2*i+0];
        pStore[2*i+0] = pStore[2*i+1];
        pStore[2*i+1] = Temp;
        Kit_TruthChangePhase( pIn, nVars, i );
    }

//    Kit_PrintHexadecimal( stdout, pIn, nVars );
//    printf( "\n" );

    // permute
    Counter = 0;
    do {
        fChange = 0;
        for ( i = 0; i < nVars-1; i++ )
        {
            if ( pStore[2*i] >= pStore[2*(i+1)] )
                continue;
            Counter++;
            fChange = 1;

            Temp = pCanonPerm[i];
            pCanonPerm[i] = pCanonPerm[i+1];
            pCanonPerm[i+1] = Temp;

            Temp = pStore[2*i];
            pStore[2*i] = pStore[2*(i+1)];
            pStore[2*(i+1)] = Temp;

            Temp = pStore[2*i+1];
            pStore[2*i+1] = pStore[2*(i+1)+1];
            pStore[2*(i+1)+1] = Temp;

            // if the polarity of variables is different, swap them
            if ( ((uCanonPhase & (1 << i)) > 0) != ((uCanonPhase & (1 << (i+1))) > 0) )
            {
                uCanonPhase ^= (1 << i);
                uCanonPhase ^= (1 << (i+1));
            }

            Kit_TruthSwapAdjacentVars( pOut, pIn, nVars, i );
            pTemp = pIn; pIn = pOut; pOut = pTemp;
        }
    } while ( fChange );


/*
    Extra_PrintBinary( stdout, &uCanonPhase, nVars+1 ); printf( " : " );
    for ( i = 0; i < nVars; i++ )
        printf( "%d=%d/%d  ", pCanonPerm[i], pStore[2*i], pStore[2*i+1] );
    printf( "  C = %d\n", Counter );
    Extra_PrintHexadecimal( stdout, pIn, nVars );
    printf( "\n" );
*/

/*
    // process symmetric variable groups
    uSymms = 0;
    for ( i = 0; i < nVars-1; i++ )
    {
        if ( pStore[2*i] != pStore[2*(i+1)] ) // i and i+1 cannot be symmetric
            continue;
        if ( pStore[2*i] != pStore[2*i+1] )
            continue;
        if ( Kit_TruthVarsSymm( pIn, nVars, i, i+1 ) )
            continue;
        if ( Kit_TruthVarsAntiSymm( pIn, nVars, i, i+1 ) )
            Kit_TruthChangePhase( pIn, nVars, i+1 );
    }
*/

/*
    // process symmetric variable groups
    uSymms = 0;
    for ( i = 0; i < nVars-1; i++ )
    {
        if ( pStore[2*i] != pStore[2*(i+1)] ) // i and i+1 cannot be symmetric
            continue;
        // i and i+1 can be symmetric
        // find the end of this group
        for ( k = i+1; k < nVars; k++ )
            if ( pStore[2*i] != pStore[2*k] ) 
                break;
        Limit = k;
        assert( i < Limit-1 );
        // go through the variables in this group
        for ( j = i + 1; j < Limit; j++ )
        {
            // check symmetry
            if ( Kit_TruthVarsSymm( pIn, nVars, i, j ) )
            {
                uSymms |= (1 << j);
                continue;
            }
            // they are phase-unknown
            if ( pStore[2*i] == pStore[2*i+1] ) 
            {
                if ( Kit_TruthVarsAntiSymm( pIn, nVars, i, j ) )
                {
                    Kit_TruthChangePhase( pIn, nVars, j );
                    uCanonPhase ^= (1 << j);
                    uSymms |= (1 << j);
                    continue;
                }
            }

            // they are not symmetric - move j as far as it goes in the group
            for ( k = j; k < Limit-1; k++ )
            {
                Counter++;

                Temp = pCanonPerm[k];
                pCanonPerm[k] = pCanonPerm[k+1];
                pCanonPerm[k+1] = Temp;

                assert( pStore[2*k] == pStore[2*(k+1)] );
                Kit_TruthSwapAdjacentVars( pOut, pIn, nVars, k );
                pTemp = pIn; pIn = pOut; pOut = pTemp;
            }
            Limit--;
            j--;
        }
        i = Limit - 1;
    }
*/

    // swap if it was moved an even number of times
    if ( Counter & 1 )
        Kit_TruthCopy( pOut, pIn, nVars );
    return uCanonPhase;
}


void Kit::Kit_TruthCountOnesInCofs( word32 * pTruth, int nVars, int * pStore )
{
    int nWords = Kit_TruthWordNum( nVars );
    int i, k, Counter;
    memset( pStore, 0, sizeof(int) * 2 * nVars );
    if ( nVars <= 5 )
    {
        if ( nVars > 0 )
        {
            pStore[2*0+0] = Kit_WordCountOnes( pTruth[0] & 0x55555555 );
            pStore[2*0+1] = Kit_WordCountOnes( pTruth[0] & 0xAAAAAAAA );
        }
        if ( nVars > 1 )
        {
            pStore[2*1+0] = Kit_WordCountOnes( pTruth[0] & 0x33333333 );
            pStore[2*1+1] = Kit_WordCountOnes( pTruth[0] & 0xCCCCCCCC );
        }
        if ( nVars > 2 )
        {
            pStore[2*2+0] = Kit_WordCountOnes( pTruth[0] & 0x0F0F0F0F );
            pStore[2*2+1] = Kit_WordCountOnes( pTruth[0] & 0xF0F0F0F0 );
        }
        if ( nVars > 3 )
        {
            pStore[2*3+0] = Kit_WordCountOnes( pTruth[0] & 0x00FF00FF );
            pStore[2*3+1] = Kit_WordCountOnes( pTruth[0] & 0xFF00FF00 );
        }
        if ( nVars > 4 )
        {
            pStore[2*4+0] = Kit_WordCountOnes( pTruth[0] & 0x0000FFFF );
            pStore[2*4+1] = Kit_WordCountOnes( pTruth[0] & 0xFFFF0000 );
        }
        return;
    }
    // nVars >= 6
    // count 1's for all other variables
    for ( k = 0; k < nWords; k++ )
    {
        Counter = Kit_WordCountOnes( pTruth[k] );
        for ( i = 5; i < nVars; i++ )
            if ( k & (1 << (i-5)) )
                pStore[2*i+1] += Counter;
            else
                pStore[2*i+0] += Counter;
    }
    // count 1's for the first five variables
    for ( k = 0; k < nWords/2; k++ )
    {
        pStore[2*0+0] += Kit_WordCountOnes( (pTruth[0] & 0x55555555) | ((pTruth[1] & 0x55555555) <<  1) );
        pStore[2*0+1] += Kit_WordCountOnes( (pTruth[0] & 0xAAAAAAAA) | ((pTruth[1] & 0xAAAAAAAA) >>  1) );
        pStore[2*1+0] += Kit_WordCountOnes( (pTruth[0] & 0x33333333) | ((pTruth[1] & 0x33333333) <<  2) );
        pStore[2*1+1] += Kit_WordCountOnes( (pTruth[0] & 0xCCCCCCCC) | ((pTruth[1] & 0xCCCCCCCC) >>  2) );
        pStore[2*2+0] += Kit_WordCountOnes( (pTruth[0] & 0x0F0F0F0F) | ((pTruth[1] & 0x0F0F0F0F) <<  4) );
        pStore[2*2+1] += Kit_WordCountOnes( (pTruth[0] & 0xF0F0F0F0) | ((pTruth[1] & 0xF0F0F0F0) >>  4) );
        pStore[2*3+0] += Kit_WordCountOnes( (pTruth[0] & 0x00FF00FF) | ((pTruth[1] & 0x00FF00FF) <<  8) );
        pStore[2*3+1] += Kit_WordCountOnes( (pTruth[0] & 0xFF00FF00) | ((pTruth[1] & 0xFF00FF00) >>  8) );
        pStore[2*4+0] += Kit_WordCountOnes( (pTruth[0] & 0x0000FFFF) | ((pTruth[1] & 0x0000FFFF) << 16) );
        pStore[2*4+1] += Kit_WordCountOnes( (pTruth[0] & 0xFFFF0000) | ((pTruth[1] & 0xFFFF0000) >> 16) );
        pTruth += 2;
    }
}



void Kit::Kit_TruthChangePhase_64bit( word64 * pInOut, int nVars, int iVar )
{
    int nWords = Kit_TruthWordNum_64bit( nVars );
    int i, Step,SizeOfBlock;
    word64 Temp[512];
    
    assert( iVar < nVars );
    if(iVar<=5)
    {
        for ( i = 0; i < nWords; i++ )
            pInOut[i] = ((pInOut[i] & mask0[iVar]) << (1<<(iVar))) | ((pInOut[i] & ~mask0[iVar]) >> (1<<(iVar)));
    }
    else
    {
        Step = (1 << (iVar - 6));
        SizeOfBlock = sizeof(word64)*Step;
        for ( i = 0; i < nWords; i += 2*Step )
        {   
            memcpy(Temp,pInOut,(size_t)SizeOfBlock);
            memcpy(pInOut,pInOut+Step,(size_t)SizeOfBlock);
            memcpy(pInOut+Step,Temp,(size_t)SizeOfBlock);
            //          Temp = pInOut[i];
            //          pInOut[i] = pInOut[Step+i];
            //          pInOut[Step+i] = Temp;          
            pInOut += 2*Step;
        }
    }
    
}

void Kit::Kit_TruthNot_64bit(word64 * pIn, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum_64bit(nVars)-1; w >= 0; w-- )
        pIn[w] = ~pIn[w];
}

void Kit::Kit_TruthCopy_64bit( word64 * pOut, word64 * pIn, int nVars )
{ 
    memcpy(pOut,pIn,Kit_TruthWordNum_64bit(nVars)*sizeof(word64));
}


void Kit::Kit_TruthSwapAdjacentVars_64bit( word64 * pInOut, int nVars, int iVar )
{
    int i, Step, Shift, SizeOfBlock;                   //
    word64 temp[256];                   // to make only pInOut possible
    static word64 PMasks[5][3] = {
        { ABCLITE_CONST_64(0x9999999999999999), ABCLITE_CONST_64(0x2222222222222222), ABCLITE_CONST_64(0x4444444444444444) },
        { ABCLITE_CONST_64(0xC3C3C3C3C3C3C3C3), ABCLITE_CONST_64(0x0C0C0C0C0C0C0C0C), ABCLITE_CONST_64(0x3030303030303030) },
        { ABCLITE_CONST_64(0xF00FF00FF00FF00F), ABCLITE_CONST_64(0x00F000F000F000F0), ABCLITE_CONST_64(0x0F000F000F000F00) },
        { ABCLITE_CONST_64(0xFF0000FFFF0000FF), ABCLITE_CONST_64(0x0000FF000000FF00), ABCLITE_CONST_64(0x00FF000000FF0000) },
        { ABCLITE_CONST_64(0xFFFF00000000FFFF), ABCLITE_CONST_64(0x00000000FFFF0000), ABCLITE_CONST_64(0x0000FFFF00000000) }
    };
    int nWords = Kit_TruthWordNum_64bit( nVars ); 
    
    assert( iVar < nVars - 1 );
    if ( iVar < 5 )
    {
        Shift = (1 << iVar);
        for ( i = 0; i < nWords; i++ )
            pInOut[i] = (pInOut[i] & PMasks[iVar][0]) | ((pInOut[i] & PMasks[iVar][1]) << Shift) | ((pInOut[i] & PMasks[iVar][2]) >> Shift);
    }
    else if ( iVar > 5 )
    {
        Step = 1 << (iVar - 6);
        SizeOfBlock = sizeof(word64)*Step;
        pInOut += 2*Step;
        for(i=2*Step; i<nWords; i+=4*Step)
        {           
            memcpy(temp,pInOut-Step,(size_t)SizeOfBlock);
            memcpy(pInOut-Step,pInOut,(size_t)SizeOfBlock);
            memcpy(pInOut,temp,(size_t)SizeOfBlock);
            pInOut += 4*Step;
        }
    }
    else // if ( iVar == 5 )
    {
        for ( i = 0; i < nWords; i += 2 )
        {
            temp[0] = pInOut[i+1] << 32;
            pInOut[i+1] ^= (temp[0] ^ pInOut[i]) >> 32;
            pInOut[i] = (pInOut[i] & 0x00000000FFFFFFFF) | temp[0];
            
        }
    }
}



unsigned  Kit::Kit_TruthSemiCanonicize_Yasha1( word64* pInOut, int nVars, char * pCanonPerm, int * pStore )
{
    int nWords = Kit_TruthWordNum_64bit( nVars );
    int i, fChange, nOnes;
    int Temp;
    unsigned  uCanonPhase=0;
    assert( nVars <= 16 );
    
    nOnes = Kit_TruthCountOnes_64bit(pInOut, nVars);
    if ( nOnes == nWords * 32 )
        uCanonPhase |= (1 << (nVars+2));
    
    else if ( (nOnes > nWords * 32) )
    {
        uCanonPhase |= (1 << nVars);
        Kit_TruthNot_64bit( pInOut, nVars );
        nOnes = nWords*64 - nOnes;
    }
    
    // collect the minterm counts
    Kit_TruthCountOnesInCofs_64bit( pInOut, nVars, pStore );
    
    // canonicize phase
    for ( i = 0; i < nVars; i++ )
    {
        if (  2*pStore[i]  == nOnes)
        {
            uCanonPhase |= (1 << (nVars+1));
            continue;
        }
        if ( pStore[i] > nOnes-pStore[i])
            continue;
        uCanonPhase |= (1 << i);
        pStore[i] = nOnes-pStore[i]; 
        Kit_TruthChangePhase_64bit( pInOut, nVars, i );
    }  
    
    do {
        fChange = 0;
        for ( i = 0; i < nVars-1; i++ )
        {
            if ( pStore[i] <= pStore[i+1] )
                continue;            
            fChange = 1;
            
            Temp = pCanonPerm[i];
            pCanonPerm[i] = pCanonPerm[i+1];
            pCanonPerm[i+1] = Temp;
            
            Temp = pStore[i];
            pStore[i] = pStore[i+1];
            pStore[i+1] = Temp;
            
            // if the polarity of variables is different, swap them
            if ( ((uCanonPhase & (1 << i)) > 0) != ((uCanonPhase & (1 << (i+1))) > 0) )
            {
                uCanonPhase ^= (1 << i);
                uCanonPhase ^= (1 << (i+1));
            }
            
            Kit_TruthSwapAdjacentVars_64bit( pInOut, nVars, i );            
        }
    } while ( fChange );
    return uCanonPhase;
}

