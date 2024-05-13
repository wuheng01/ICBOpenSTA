#pragma once

#include <vector>
#include <Cut.hh>

namespace sta {

  class Network;
  class Pin;
  class Instance;
  class Cut;
  
class CutGen {
public:
  using expansionSet=std::vector<std::pair<std::vector<const Pin*>, std::vector<const Pin*> > >;

  CutGen(Network* nwk, LibertyLibrary* liberty_lib): network_(nwk),liberty_lib_(liberty_lib){}
  void GenerateInstanceCutSet(Instance*,std::vector<Cut*>&);
  bool EnumerateExpansions(std::vector<const Pin*>& candidate_wavefront,
			   expansionSet& expansion_set);
  Cut* BuildCutFromExpansionElement(Instance* cur_inst,const Pin* root_pin,
				    std::vector<const Pin*>& keep, //set of pins to keep
				    std::vector<const Pin*>& expand);
  bool Boundary(const Pin* cur_pin);
  const Pin* WalkThroughBuffersAndInverters(Cut*,
					    const Pin* op_pin,
					    std::set<const Pin*>& unique_set);
  
private:
  Network* network_;
  LibertyLibrary* liberty_lib_;
};

}
