#pragma once
#include "FuncExpr.hh"
#include "StringUtil.hh"
#include "Liberty.hh"
#include "Network.hh"


namespace sta {

  class FuncExpr2Kit {
  public:    
    FuncExpr2Kit(FuncExpr* fe, int& var_count, unsigned *&kit_tables);
    void RecursivelyEvaluate(FuncExpr* fe, unsigned*);

  private:
    std::map<LibertyPort*, unsigned*> kit_vars_; //pointer (not name) addressed.
    std::map<int,LibertyPort*>  lib_ports_;
    int var_count_;
  };
  
}
