#pragma once

#include <vector>
#include <Cut.hh>
#include <base/abc/abc.h>


namespace sta {
  
  class Cut;
  class ConcreteNetwork;
  
class Ntk2Cut {
public:
  Ntk2Cut(abc::Abc_Ntk_t*, Cut*, ConcreteNetwork*);
  void BuildNwkElements();

private:
  abc::Abc_Ntk_t* mapped_netlist_;
  Cut* cut_;
  ConcreteNetwork* target_nwk_;
};

}
