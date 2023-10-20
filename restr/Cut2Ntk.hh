#pragma once

#include <vector>
#include <Cut.hh>
#include <base/abc/abc.h>

namespace abc {
typedef struct Abc_Ntk_t_ Abc_Ntk_t;
typedef struct Abc_Obj_t_ Abc_Obj_t;
}

namespace sta {
  
  class Cut2Ntk {
  public:
    Cut2Ntk(Network*  nwk, Cut* cut, std::string cut_name):sta_nwk_(nwk),
							   cut_(cut),
							   cut_name_(cut_name){}
    abc::Abc_Ntk_t* BuildSopLogicNetlist();
    abc::Abc_Obj_t* CreateGate(const Instance* cur_inst);
    static bool SopNwkCheck(abc::Abc_Ntk_t* abc_nwk);
    void CleanUpNetNets();
    
  private:
    Network* sta_nwk_;    
    Cut* cut_;
    std::string cut_name_;

    abc::Abc_Ntk_t* abc_nwk_;
    //cross reference dictionaries for ease of constraining.
    std::map<const Pin*, abc::Abc_Obj_t*> pi_table_;
    std::map<const Pin*, abc::Abc_Obj_t*> po_table_;    
    std::map<const Pin*, abc::Abc_Obj_t*>  net_table_sta2abc_; //sta -> abc
    std::map<abc::Abc_Obj_t*, const Pin*>  net_table_abc2sta_; //abc -> sta.
  };
}
