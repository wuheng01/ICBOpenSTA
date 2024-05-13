/*
Unit tests for npn encode.
Shows the interface for getting the npn signature for a 
word.
*/

#include <gtest/gtest.h>
#include "ExtraUtil.hh"
#include "Npnencode.hh"
#include "Kit.hh"
#include <cassert>

class NpnEncodeTest: public ::testing::Test {
protected:
  void SetUp() override {
  }
  void TearDown() override {}
};

TEST_F(NpnEncodeTest, AndNpn1){
  word64 result;
  word64 ip;
  ip = 8;
  char pCanonPerm[16];
  unsigned uCanonPhase;
  Npnencode::resetPCanonPermArray(pCanonPerm, 3);
  uCanonPhase = Npnencode::luckyCanonicizer_final_fast(&ip,3, pCanonPerm);
  EXPECT_EQ(ip,1);
}


TEST_F(NpnEncodeTest, AndNpn2){
  word64 result;
  word64 ip;
  ip = 0x02;

/*
Note this is truth_table

210 <- column index

000 0
001 1
010 0
011 0

100 0
101 0
110 0
111 0

0x02

The required result is 0x01.

To make this we need to invert colum 0, that will push the 001 -> 000

*/
  
  char pCanonPerm[16];
  unsigned uCanonPhase=0;
  Npnencode::resetPCanonPermArray(pCanonPerm, 3);
  uCanonPhase = Npnencode::luckyCanonicizer_final_fast(&ip,3, pCanonPerm);

  std::stringstream perm_ss;
  std::stringstream phase_ss;  
  
  for (int i = 0; i < 8; i++){
    if ( (pCanonPerm[i] >= 'a') &&
	 (pCanonPerm[i] - 'a' < 3)){
      char temp[10];
      temp[0] = 'i';
      temp[1] = '_';
      temp[2] = pCanonPerm[i] - 'a' + '0';
      temp[3] = '\0';
      perm_ss << temp << " ";
      if (uCanonPhase & (0x1 << i)){
	temp[0] = 'i';
	temp[1] = '_';
	temp[2] = pCanonPerm[i] - 'a' + '0';
	temp[3] = ' ';
	//1 means negated
	temp[4] = (uCanonPhase & (0x1 << i)) ? 'F':'T';
	temp[5] = '\0';
	phase_ss << temp << " ";
      }
    }
  }
  //  printf("Perm %s\n", perm_ss.str().c_str());
  //  printf("Phase %s\n", phase_ss.str().c_str());
  
  EXPECT_EQ(ip,1);
}


TEST_F(NpnEncodeTest, OrNpn1){
  word64 result;
  word64 ip;
  ip = 0xfe;
  /*
Note this is truth_table

000 0
001 1
010 1
011 1

100 1
101 1
110 1
111 1

FE

The smallest number with 7 ones is
0111_1111
7F.

This is made by inverting all the inputs
*/
  char pCanonPerm[16];
  unsigned uCanonPhase;
  Npnencode::resetPCanonPermArray(pCanonPerm, 3);
  std::stringstream perm_ss;
  std::stringstream phase_ss;  
  uCanonPhase =Npnencode::luckyCanonicizer_final_fast(&ip,3, pCanonPerm);

  //Construct string for permutation
  //  printf("\nPerms\n");
  for (int i = 0; i < 8; i++){
    if ( (pCanonPerm[i] >= 'a') &&
	 (pCanonPerm[i] - 'a' < 3)){
      char temp[10];
      temp[0] = 'i';
      temp[1] = '_';
      temp[2] = pCanonPerm[i] - 'a' + '0';
      temp[3] = '\0';
      perm_ss << temp << " ";

      if (uCanonPhase & (0x1 << i)){
	temp[0] = 'i';
	temp[1] = '_';
	temp[2] = pCanonPerm[i] - 'a' + '0';
	temp[3] = ' ';
	//1 means negated
	temp[4] = (uCanonPhase & (0x1 << i)) ? 'F':'T';
	temp[5] = '\0';
	phase_ss << temp << " ";
      }
    }
  }

  EXPECT_EQ(ip,0x7f);
  EXPECT_EQ(perm_ss.str(),"i_1 i_2 i_0 ");
  EXPECT_EQ(phase_ss.str(),"i_1 F i_2 F i_0 F ");
}


TEST_F(NpnEncodeTest, OrNpn2){
  word64 result;
  word64 ip;
  ip = 0xf7;
  /*
Note this is truth_table

210 <- column index

000 1
001 1
010 1
011 0

100 1
101 1
110 1
111 1

F7

The smallest number with 7 ones is
0111_1111

7F.



And we permute the columns into order
120

Permuted table
120    Op
000    1    
001    1
100    1
101    1
010    1
011    0 <- note how the point 011 has moved
110    1
111    1

To move 011 -> point 11 we need to invert column 1
Inverting column 1:

-
120
100    1    
101    1
000    1
001    1
110    1
111    1
010    1
011    0 <- note how we have moved the 0 to msb

Result:
0x7F.
*/
  char pCanonPerm[16];
  unsigned uCanonPhase;
  Npnencode::resetPCanonPermArray(pCanonPerm, 3);
  std::stringstream perm_ss;
  std::stringstream phase_ss;  
  uCanonPhase =Npnencode::luckyCanonicizer_final_fast(&ip,3, pCanonPerm);

  //Construct string for permutation
  //  printf("\nPerms\n");
  for (int i = 0; i < 8; i++){
    if ( (pCanonPerm[i] >= 'a') &&
	 (pCanonPerm[i] - 'a' < 3)){
      char temp[10];
      temp[0] = 'i';
      temp[1] = '_';
      temp[2] = pCanonPerm[i] - 'a' + '0';
      temp[3] = '\0';
      perm_ss << temp << " ";
      //printf("%s \n", temp);
      if (uCanonPhase & (0x1 << i)){
	temp[0] = 'i';
	temp[1] = '_';
	temp[2] = pCanonPerm[i] - 'a' + '0';
	temp[3] = ' ';
	//1 means negated
	temp[4] = (uCanonPhase & (0x1 << i)) ? 'F':'T';
	temp[5] = '\0';
	phase_ss << temp << " ";
      }
    }
  }
  //    printf("Perm %s\n", perm_ss.str().c_str());
  //    printf("Phase %s\n", phase_ss.str().c_str());
  EXPECT_EQ(ip,0x7f);
  EXPECT_EQ(perm_ss.str(),"i_1 i_2 i_0 ");
  EXPECT_EQ(phase_ss.str(),"i_2 F ");
  }


TEST_F(NpnEncodeTest, XorNpn2){
  word64 result;
  word64 ip;
  ip = 0x6;
  char pCanonPerm[16];
  unsigned uCanonPhase;
  Npnencode::resetPCanonPermArray(pCanonPerm, 2);
  Npnencode::luckyCanonicizer_final_fast(&ip,2, pCanonPerm);
  EXPECT_EQ(ip,0x6);
}

TEST_F(NpnEncodeTest, XorNpn3){
  word64 result;
  word64 ip;
  ip = 0x96;
  char pCanonPerm[16];
  unsigned uCanonPhase;
  Npnencode::resetPCanonPermArray(pCanonPerm, 3);
  Npnencode::luckyCanonicizer_final_fast(&ip,3, pCanonPerm);
  EXPECT_EQ(ip,0x69);
}


int main(int argc, char** argv){
  ::testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
