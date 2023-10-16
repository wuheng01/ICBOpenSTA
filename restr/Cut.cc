#include <cassert>
#include <sstream>
#include <functional>
#include <vector>
#include <map>

#include "liberty/LibertyBuilder.hh"
#include "PortDirection.hh"
#include "Network.hh"
#include "Liberty.hh"

#include "FuncExpr.hh"
#include "FuncExpr2Kit.hh"
#include "Cut.hh"

#include "ExtraUtil.hh"
#include "Npnencode.hh"




namespace sta {

  //
  //Build the FuncExpr for a cut
  //
  
  void Cut::BuildFuncExpr(Network* nwk,
			  LibertyLibrary* cut_library){

    std::string cut_name = "cut_" + std::to_string(id_);
    
    //we are building new liberty ports. We iterate them
    
    LibertyBuilder lib_builder;
    cut_cell_ = lib_builder.makeCell(cut_library,
				     cut_name.c_str(),
				     nullptr);
    
    std::map<const Pin*,LibertyPort*> leaf_pin_to_port;
    std::map<const Pin*,FuncExpr*> visited_pins;    
    for (auto i: root_){
      LibertyPort* lp = nwk -> libertyPort(i);
      if (lp && lp -> function()){
	FuncExpr* lp_fn = lp -> function();
	FuncExpr* fe = ExpandFunctionExprR(nwk,
					   i, 
					   lp_fn,
					   leaf_pin_to_port,
					   visited_pins,
					   lib_builder
					   );
	root_fns_.push_back(fe);
      }
    }
  }

  


  bool
  Cut::Boundary(Network* network,
		 const Pin* cur_pin){
    //a terminal
    if (network -> term(cur_pin))
      return true;
    //a memory
    Instance* cur_inst = network -> instance(cur_pin);
    if (cur_inst){
      LibertyCell* cell = network -> libertyCell(cur_inst);
      if (cell -> isMemory() ||
	  cell -> hasSequentials())
	return true;
    }
    return false;
  }
  

  FuncExpr*
  Cut::BuildFuncExprR(Network* nwk,
		      Instance* cur_inst,
		      const Pin* op_pin,
		      FuncExpr* op_pin_fe,
		      std::map<const Pin*,LibertyPort*>& leaf_pin2port,
		      std::map<const Pin*, FuncExpr*>& visited_pins,
		      LibertyBuilder &lib_builder){
    FuncExpr* ret = nullptr;

    
    if (op_pin && (visited_pins.find(op_pin)!=
		   visited_pins.end()))
      return visited_pins[op_pin];
    
    if (op_pin_fe){
      switch (op_pin_fe -> op()){

      case FuncExpr::op_port:
	{
	  
	  LibertyPort* lp = op_pin_fe -> port();	  


	  if (op_pin == nullptr){
	    //fish out the port on the cell
	    InstancePinIterator *pin_iter = nwk -> pinIterator(cur_inst);
	    while (pin_iter -> hasNext()){
	      const Pin* ip_pin = pin_iter -> next();
	      if (nwk -> direction(ip_pin) -> isInput()){
		LibertyPort* candidate_lp = nwk -> libertyPort(ip_pin);
		if (candidate_lp == lp){
		  op_pin = ip_pin;
		}
	      }
	    }
	  }

	  assert(op_pin);
	  
	  if ((visited_pins.find(op_pin)!=
	       visited_pins.end()))
	    return visited_pins[op_pin];
    

	  if (LeafPin(op_pin)){
	    FuncExpr* ret = op_pin_fe -> copy();
	    visited_pins[op_pin] = ret;
	    leaf_pin2port[op_pin]=lp;
	    return ret;
	  }
	    
	  PinSet* drvrs = nwk -> drivers(op_pin);
	  if (drvrs){
	    PinSet::Iterator drvr_iter(drvrs);
	    const Pin* driving_pin = drvr_iter.next();
	    //
	    //driving pin is a leaf.
	    //
	    if (LeafPin(driving_pin)){
	      LibertyPort* dup_port= lib_builder.makePort(cut_cell_, lp -> name());
	      FuncExpr* ret = FuncExpr::makePort(dup_port);
	      visited_pins[op_pin] = ret;
	      leaf_pin2port[op_pin]= dup_port;
	      return ret;
	    }
	    LibertyPort* driving_lp = nwk -> libertyPort(driving_pin);
	    if (driving_lp){
	      Instance* driving_inst = nwk -> instance(driving_pin);
	      FuncExpr* op_fe = BuildFuncExprR(nwk,
					       driving_inst,
					       driving_pin,
					       driving_lp -> function(),
					       leaf_pin2port,
					       visited_pins,
					       lib_builder);
	      visited_pins[driving_pin] = op_fe;
	      return op_fe;
	    }
	  }
	}
	break;
	
      case FuncExpr::op_not:
	{
	  //left
	  FuncExpr* arg = BuildFuncExprR(nwk,
					 cur_inst,
					 nullptr,
					 op_pin_fe -> left(),
					 leaf_pin2port,
					 visited_pins,
					 lib_builder);
	  return FuncExpr::makeNot(arg);
	}
	break;
      case FuncExpr::op_and:
	{
	  FuncExpr* left = BuildFuncExprR(nwk,
					  cur_inst,
					  nullptr,
					  op_pin_fe -> left(),
					  leaf_pin2port,
					  visited_pins,
					  lib_builder);
	  
	  FuncExpr* right = BuildFuncExprR(nwk,
					   cur_inst,
					   nullptr,
					   op_pin_fe -> right(),
					   leaf_pin2port,
					   visited_pins,
					   lib_builder);
	  
	  FuncExpr* ret = FuncExpr::makeAnd(left,right);
	  return ret;
	}
	break;
      case FuncExpr::op_or:
	{
	  FuncExpr* left = BuildFuncExprR(nwk,
					  cur_inst,
					  nullptr,
					  op_pin_fe -> left(),
					  leaf_pin2port,
					  visited_pins,
					  lib_builder);
	  
	  FuncExpr* right = BuildFuncExprR(nwk,
					   cur_inst,
					   nullptr,
					   op_pin_fe -> right(),
					   leaf_pin2port,
					   visited_pins,
					   lib_builder);
	  
	  FuncExpr* ret = FuncExpr::makeOr(left,right);
	  return ret;
	}
	break;
      default:
	return nullptr;
      }
    }
    return ret;
  }


  

  bool
  Cut::LeafPin( const Pin* cur_pin){
    for (auto i: leaves_)
      if (i == cur_pin)
	return true;
    return false;
  }
	       
  
  FuncExpr*
  Cut::ExpandFunctionExprR(Network* nwk,
			   const Pin* op_pin,
			   FuncExpr* op_pin_fe,
			   std::map<const Pin*,LibertyPort*>& leaf_pin_to_port,
			   std::map<const Pin*, FuncExpr*>& visited,
			   LibertyBuilder& lib_builder
			   ){

    if (op_pin && (visited.find(op_pin)!= visited.end())){
      return visited[op_pin];
    }
    
    Instance* cur_inst = nwk -> instance(op_pin);
    InstancePinIterator *pin_iter = nwk -> pinIterator(cur_inst);

    //generate logic functions for each input
    while (pin_iter -> hasNext()){
      const Pin* ip_pin = pin_iter -> next();
      if (nwk -> direction(ip_pin) -> isInput()){
	PinSet* drvrs = nwk -> drivers(ip_pin);
	if (drvrs){
	  PinSet::Iterator drvr_iter(drvrs);	  
	  if (drvr_iter.hasNext()){
	    const Pin* cur_pin = drvr_iter.next();
	    //stop on leaf pins
	    if (LeafPin(cur_pin)){
	      LibertyPort* lp_pin = nwk -> libertyPort(ip_pin);
	      leaf_pin_to_port[cur_pin]=lp_pin;
	    }
	    //push back through internal pins
	    else if (nwk -> direction(cur_pin) -> isOutput()){	    
	      if (internal_pins_.find(cur_pin)!=
		  internal_pins_.end()){
		LibertyPort* lp = nwk -> libertyPort(cur_pin);
		if (lp && lp -> function()){
		  FuncExpr *cur_pin_fn = ExpandFunctionExprR(nwk,
							     cur_pin,
							     lp -> function(),
							     leaf_pin_to_port,
							     visited,
							     lib_builder
);
		  //register the internal pin function
		  visited[cur_pin]= cur_pin_fn;
		}
	      }
	    }
	  }
	}
      }
    }
    //make function for gate from inputs
    FuncExpr* gate_fn = BuildFuncExprR(nwk,
				       cur_inst,
				       op_pin,
				       op_pin_fe,
				       leaf_pin_to_port,
				       visited,
				       lib_builder);
    visited[op_pin] = gate_fn;

    return gate_fn;
  }

  

  void Cut::Print(FILE* fp, Network* nwk){

    fprintf(fp,"Cut: %d\n",id_);
    fprintf(fp,"+++\n");
    fprintf(fp,"Roots:\n");
    unsigned ix=0;
    
    for (auto r: root_){
      fprintf(fp,"Root: %s %s (%p) dir %s\n",
	      nwk -> pathName(r),
	      root_fns_[ix] ? root_fns_[ix] -> asString():"null",
	      root_fns_[ix],
	      nwk -> direction(r) -> isInput() ? " I ": " O "
	      );
      int var_count;
      unsigned* kit_table=nullptr;
      FuncExpr2Kit(root_fns_[ix],var_count,kit_table);
      unsigned num_bits_to_set = (0x01 << var_count );			  
      unsigned mask=0x00;
      for (unsigned j=0; j < num_bits_to_set; j++)
	mask = mask | (0x1<< j);
      unsigned kit_result = *kit_table & mask;
      fprintf(fp,"\n T-table: ");
      fprintf(fp," (0x%x) var count %d\n",kit_result,var_count);

      Npnencode::TtStore_t* ttstore = Npnencode::TruthStoreAlloc(var_count,1);
      if (ttstore){
	word64 kit_result_ulong = kit_result;
	ttstore -> pFuncs[0] = &kit_result_ulong;
	char pCanonPerm[16];
	unsigned uCanonPhase;
	//We only have one function !
	for ( int i = 0; i < ttstore ->nFuncs; i++ )   {
	  //reset the data structure
	  Npnencode::resetPCanonPermArray(pCanonPerm, ttstore->nVars);
	  //Invoke the npn
	  word64  ull = *(ttstore -> pFuncs[i]);
	  uCanonPhase = Npnencode::luckyCanonicizer_final_fast(/*ttstore -> pFuncs[i]*/&ull, ttstore->nVars, pCanonPerm );
	  fprintf(fp,"NPN Signature: ");
	  std::stringstream ss;
	  fprintf(fp, "0x%x\n",ull);
	  //	  ExtraUtil::PrintHex( ss, (unsigned *)ull, ttstore->nVars );
	  //	  fprintf(fp,"0x%s\n",ss.str().c_str());
	  fflush(fp);			  
	}
      ix++;
    }
    }
    fflush(fp);
    fprintf(fp,"Leaves:\n");
    for (auto l: leaves_)
      fprintf(fp,"Leaf: %s dirn %s\n", nwk -> pathName(l),
	      nwk -> direction(l) -> isInput() ? " I " : " O "
	      );
    fflush(fp);    
    fprintf(fp,"Volume:\n");
    for (auto v: volume_){
      fprintf(fp,"V: %s (%s) ", nwk -> pathName(v), nwk -> libertyCell(v) ?
	      nwk -> libertyCell(v)-> name(): "unk" );
      fflush(fp);
      InstancePinIterator *cpin_iter = nwk -> pinIterator(v);
      while (cpin_iter -> hasNext()){
	const Pin *c_pin = cpin_iter -> next();
	if (nwk -> direction(c_pin) -> isOutput()){
	  LibertyPort *lp =  nwk -> libertyPort(c_pin);
	  if (lp && lp -> function()){
	    fprintf(fp,"\t%s ",lp -> function() -> asString());
	    int var_count;
	    unsigned* kit_table=nullptr;
	    FuncExpr2Kit(lp -> function(),var_count,kit_table);
	    unsigned num_bits_to_set = (0x01 << var_count );			  
	    unsigned mask=0x00;
	    for (unsigned j=0; j < num_bits_to_set; j++)
	      mask = mask | (0x1<< j);
	    unsigned result = *kit_table & mask;
	    fprintf(fp," (0x%x) (vars: %d) \n",result,var_count);
	  }
	}
      }
    }
    fflush(fp);
    fprintf(fp, "Internal Pins\n");
    for (auto ip: internal_pins_){
      const Pin* cur_pin = ip.first;
      LibertyPort* lp = ip.second;
      fprintf(fp,"Pin %s Liberty Port %s\n",
	      nwk -> pathName(cur_pin),
	      lp -> name());
    }
    fflush(fp);
    fprintf(fp,"---\n");
  }
  
}
