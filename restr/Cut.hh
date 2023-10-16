#pragma once

#include <functional>

#include "Map.hh"
#include "StringUtil.hh"
#include "Liberty.hh"
#include "VertexId.hh"
#include "Network.hh"
#include "StaState.hh"
#include "FuncExpr.hh"
#include "liberty/LibertyBuilder.hh"

namespace sta {

/*
Cut Set representation
*/

class Cut {
  
public:
  void Print(FILE* fp, Network* nwk);
  
  //for getting logic representation of cut
  void BuildFuncExpr(Network*, LibertyLibrary* cut_library );
  bool Boundary(Network*, const Pin*);
  bool LeafPin(const Pin*);
  FuncExpr* ExpandFunctionExprR(Network* nwk,
				const Pin* op_pin,
				FuncExpr* fe,
				std::map< const Pin*,LibertyPort* > &leaf_pin_to_port,
				std::map<const Pin*,FuncExpr*>& visited_pins    			   ,
				LibertyBuilder& lib_builder
			   );
  
  FuncExpr* BuildFuncExprR(Network* nwk,
			   Instance* cur_inst,
			   const Pin* op_pin,
			   FuncExpr* op_pin_fe,
			   std::map< const Pin*, LibertyPort* > &leaf_pin_to_port,
			   std::map<const Pin*, FuncExpr* > &visited_pins,
			   LibertyBuilder &lib_builder
			   );


  std::vector<const Pin*> root_; //The set of roots (multiple roots ok).
  std::vector<const Pin*> leaves_;
  std::vector<const Instance*> volume_;
  std::map<const Pin*,LibertyPort*> internal_pins_;


  std::vector<FuncExpr*> root_fns_;
  std::vector<FuncExpr*> leaves_fns_;
  unsigned id_;
  unsigned unique_id;
  LibertyCell* cut_cell_;//liberty cell representation of cut.

  friend class PhysTiming;
  friend class Ntk2Cut;
  friend class ClusterCutGen;
};

}
