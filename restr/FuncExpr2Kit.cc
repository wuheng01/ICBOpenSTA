/*
Convert a functional expression in Timing Analyzer
to a Kit data structure suitable for abc transformation
*/

#include <abcglobal.hh>
#include <FuncExpr.hh>
#include <StringUtil.hh>
#include <Liberty.hh>
#include <Network.hh>
#include <Kit.hh>
#include <FuncExpr2Kit.hh>

namespace sta {

  FuncExpr2Kit::FuncExpr2Kit(FuncExpr* fe, int& var_count, unsigned *&kit_table){
    var_count_ =0;

    FuncExprPortIterator iter(fe);
    while (iter.hasNext()){
      LibertyPort* lp = iter.next();
      lib_ports_[var_count_]=lp;
      var_count_++;
    }
    var_count = var_count_;
    
    for (int i=0; i < var_count_; i++){
      int nWords = Kit::Kit_TruthWordNum(var_count_);
      unsigned*  pTruth= ABCLITE_ALLOC(unsigned, nWords);
      Kit::Kit_TruthIthVar(pTruth,var_count_,i);
      kit_vars_[lib_ports_[i]]=pTruth;					     
    }
    int nWords = Kit::Kit_TruthWordNum(var_count_);
    kit_table= ABCLITE_ALLOC(unsigned, nWords);
    RecursivelyEvaluate(fe,kit_table);
  }


  
  void FuncExpr2Kit::RecursivelyEvaluate(FuncExpr* fe,
					  unsigned* result ){

    if (fe){
      switch (fe -> op()){
	
      case FuncExpr::op_port:
	{
	LibertyPort* p = fe -> port();
	Kit::Kit_TruthCopy(result,kit_vars_[p],var_count_);
	}
	break;
	
      case FuncExpr::op_not:
	{
	  int nWords = Kit::Kit_TruthWordNum(var_count_);
	  unsigned*  result_left= ABCLITE_ALLOC(unsigned, nWords);
	  RecursivelyEvaluate(fe -> left(),result_left);
	  Kit::Kit_TruthNot(result,result_left, var_count_);
	  ABCLITE_FREE(result_left);
	}
	break;

      case FuncExpr::op_or:
	{
	  int nWords = Kit::Kit_TruthWordNum(var_count_);
	  unsigned*  result_left= ABCLITE_ALLOC(unsigned, nWords);
	  unsigned*  result_right= ABCLITE_ALLOC(unsigned, nWords);	  
	  RecursivelyEvaluate(fe -> left(),result_left);
	  RecursivelyEvaluate(fe -> right(),result_right);	  
	  Kit::Kit_TruthOr(result,result_left, result_right,var_count_);
	  ABCLITE_FREE(result_left);
	  ABCLITE_FREE(result_right);	  
	}
	break;
      case FuncExpr::op_and:
	{
	  int nWords = Kit::Kit_TruthWordNum(var_count_);
	  unsigned*  result_left= ABCLITE_ALLOC(unsigned, nWords);
	  unsigned*  result_right= ABCLITE_ALLOC(unsigned, nWords);	  
	  RecursivelyEvaluate(fe -> left(),result_left);
	  RecursivelyEvaluate(fe -> right(),result_right);	  
	  Kit::Kit_TruthAnd(result,result_left, result_right,var_count_);
	  ABCLITE_FREE(result_left);
	  ABCLITE_FREE(result_right);	  
	}
	break;
      case FuncExpr::op_xor:
	{
	  int nWords = Kit::Kit_TruthWordNum(var_count_);
	  unsigned*  result_left= ABCLITE_ALLOC(unsigned, nWords);
	  unsigned*  result_right= ABCLITE_ALLOC(unsigned, nWords);	  
	  RecursivelyEvaluate(fe -> left(),result_left);
	  RecursivelyEvaluate(fe -> right(),result_right);	  
	  Kit::Kit_TruthXor(result,result_left, result_right,var_count_);
	  ABCLITE_FREE(result_left);
	  ABCLITE_FREE(result_right);
	}
	break;
      case FuncExpr::op_one:
	{
	  int nWords = Kit::Kit_TruthWordNum(var_count_);
	  unsigned*  const_zero= ABCLITE_ALLOC(unsigned, nWords);
	  Kit::Kit_TruthFill(const_zero,var_count_);
	  Kit::Kit_TruthNot(result,const_zero,var_count_);
	  ABCLITE_FREE(const_zero);
	}
	break;
	
      case FuncExpr::op_zero:
	{
	  Kit::Kit_TruthFill(result,var_count_);
	}
	break;
      }
    }
  }
}
