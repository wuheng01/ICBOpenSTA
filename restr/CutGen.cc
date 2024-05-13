/*
Cut Generator
*/

/*
 */
#include "Liberty.hh"
#include "PortDirection.hh"
#include "Network.hh"
#include <Cut.hh>
#include <CutGen.hh>
#include <FuncExpr2Kit.hh>
#include <Kit.hh>
#include <Cut2Ntk.hh>

//#define CUTGEN_DEBUG 1
namespace sta {

void
CutGen::GenerateInstanceCutSet(Instance* cur_inst, std::vector<Cut*>& inst_cut_set){
  //build candidate wave front for cut enumeration
  std::vector<const Pin*> candidate_wavefront;				       
  InstancePinIterator *cpin_iter = network_ -> pinIterator(cur_inst);
  const Pin* root_pin=nullptr;
  while (cpin_iter -> hasNext()){
    const Pin *c_pin = cpin_iter -> next();
    if (Boundary(c_pin))
      continue;
    if (network_ -> direction(c_pin) -> isInput())
      candidate_wavefront.push_back(c_pin);
    else
      root_pin = c_pin;
  }

  //Expand the wave front: Keep set, Expand Set
  expansionSet expansion_set;
  EnumerateExpansions(candidate_wavefront,
		      expansion_set);
  //Build cut for each expansion
  for (auto exp_el: expansion_set){
    std::vector<const Pin*> keep = exp_el.first;
    std::vector<const Pin*> expand = exp_el.second;
    Cut* cur_cut = BuildCutFromExpansionElement(cur_inst,
						root_pin,
						keep,
						expand);
    inst_cut_set.push_back(cur_cut);
  }
}


  
  Cut*
  CutGen::BuildCutFromExpansionElement(
				       Instance* cur_inst,
				       const Pin* root_pin,
				       std::vector<const Pin*>& keep, //set of pins to keep
				       std::vector<const Pin*>& expand){
	
    std::set<const Pin*> unique_driver_pins;
    std::set<Instance*> unique_insts;
    
    static int id;
    id++;

    Cut* ret = new Cut();
    ret -> id_ = id;
#ifdef CUTGEN_DEBUG    
    printf("Building cut %d from root %s\n",id, network_ -> pathName(root_pin));
    printf("Keep set:\n");
    for (auto k:keep){
      printf("%s \n", network_ -> pathName(k));
    }
    printf("Expand set:\n");
    for (auto e:expand){
      printf("%s \n", network_ -> pathName(e));
    }
#endif
    
    ret -> root_.push_back(root_pin);
    for (auto k:keep){
      //get the drivers..
      PinSet* drvrs = network_ -> drivers(k);
      if (drvrs){
	PinSet::Iterator drvr_iter(drvrs);
	if (drvr_iter.hasNext()){
	  const Pin* driving_pin = drvr_iter.next();
	  if (unique_driver_pins.find(driving_pin) ==
	      unique_driver_pins.end()){
#ifdef CUTGEN_DEBUG    	    
	    printf("Registering driver pin %s\n",
		   network_ -> pathName(driving_pin));
#endif	    
	    ret -> leaves_.push_back(driving_pin);
	    unique_driver_pins.insert(driving_pin);
	  }
	}
      }
    }
   
    ret -> volume_.push_back(cur_inst);
    unique_insts.insert(cur_inst);

   
    for (auto e: expand){
      PinSet* drvrs = network_ -> drivers(e);
      if (drvrs){
	PinSet::Iterator drvr_iter(drvrs);
	if (drvr_iter.hasNext()){
	  const Pin* driving_pin = drvr_iter.next();

	  //filter: a boundary (eg memory, or primary i/o)
	  if (Boundary(driving_pin)){
	    if (unique_driver_pins.find(driving_pin) ==
		unique_driver_pins.end()){
	      unique_driver_pins.insert(driving_pin);
	      ret -> leaves_.push_back(driving_pin);
	    }
	    continue;	    
	  }
	  
	  if (unique_driver_pins.find(driving_pin) ==
	      unique_driver_pins.end()){
	    unique_driver_pins.insert(driving_pin);
#ifdef CUTGEN_DEBUG    
	    printf("Expanding driving pin %s\n",
		   network_ -> pathName(driving_pin));
#endif	    

	    //expand through buffers and inverters
	    driving_pin = WalkThroughBuffersAndInverters(ret,driving_pin,unique_driver_pins);
	    
	    if (network_ -> direction(driving_pin) -> isOutput()){
	      LibertyPort *lp =  network_ -> libertyPort(driving_pin);
	      ret -> internal_pins_[driving_pin] = lp;
	    }
	   
	    //get the driving instance
	    Instance* driving_inst = network_ -> instance(driving_pin);
#ifdef CUTGEN_DEBUG    	    
	    printf("Expanding driving instance %s\n",
		   network_ -> pathName(driving_inst));
#endif	    

	    //add to volume, uniquely
	    if (unique_insts.find(driving_inst) ==
		unique_insts.end()){
	      unique_insts.insert(driving_inst);
	      ret -> volume_.push_back(driving_inst);
	      InstancePinIterator *pin_iter = network_ -> pinIterator(driving_inst);
	      while (pin_iter -> hasNext()){
		const Pin* dpin = pin_iter -> next();
		if (network_ ->direction(dpin) -> isInput()){
#ifdef CUTGEN_DEBUG    		  
		  printf("Expanding through driving instance ip %s\n",
			 network_ -> pathName(dpin));
#endif		  
		  //get the driver add to cut leaves
		  PinSet* drvrs = network_ -> drivers(dpin);
		  if (drvrs){
		    PinSet::Iterator drvr_iter(drvrs);	  
		    if (drvr_iter.hasNext()){
		      const Pin* cur_pin = drvr_iter.next();
		      
		      //could be a primary input pin or an output pin
		      if (network_ -> direction(cur_pin) -> isOutput() ||
			  network_ -> term(cur_pin)){
#ifdef CUTGEN_DEBUG    			
			printf("Looking at driving pin %s\n",
			       network_ -> pathName(cur_pin));
#endif			
			if (unique_driver_pins.find(cur_pin) ==
			    unique_driver_pins.end()){
			  ret -> leaves_.push_back(cur_pin);
			  unique_driver_pins.insert(cur_pin);
#ifdef CUTGEN_DEBUG    			  
			  printf("Registering unique ip pin %s\n",
			       network_ -> pathName(cur_pin));
#endif			  
			}
		      }
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }
    ret -> BuildFuncExpr(network_,liberty_lib_);
    //Test the new code: build an abc netlist for a cut.
    //    std::string cut_name = "cut_" + std::to_string(ret -> id_);
    //    Cut2Ntk c2_abc (network_,ret,cut_name);
    //    c2_abc.BuildSopNetlist();
    
    return ret;
  }


  //
  //walk through a driving pin
  //
const Pin*
CutGen::WalkThroughBuffersAndInverters(Cut* cut,
				       const Pin* op_pin,
				       std::set<const Pin*>& unique_set){
  Instance* cur_inst = network_ -> instance(op_pin);
  InstancePinIterator *cpin_iter = network_ -> pinIterator(cur_inst);
  int var_count=0;
  const Pin* ip_pin;
  while (cpin_iter -> hasNext()){
    const Pin *c_pin = cpin_iter -> next();
    if (network_ -> direction(c_pin) -> isInput()){
      var_count++;
      ip_pin = c_pin;
    }
  }
  if (var_count == 1){
    LibertyPort* lp = network_ -> libertyPort(op_pin);
    if (lp && lp -> function()){
      unsigned* kit_table = nullptr;
      FuncExpr2Kit FuncExpr2Kit(lp -> function(), var_count, kit_table);
            unsigned num_bits_to_set = (0x01 << var_count );			        
      unsigned mask=0x00;
      for (unsigned j=0; j < num_bits_to_set; j++)
	mask = mask | (0x1<< j);
      unsigned result = *kit_table & mask;

      if (Kit::Kit_IsBuffer(kit_table,var_count) ||
	  Kit::Kit_IsInverter(kit_table,var_count)){
	if (ip_pin){
#ifdef CUTGEN_DEBUG    	  
	  printf("Walking through buffer/inverter\n");
#endif	  
	  PinSet* drvrs = network_ -> drivers(ip_pin);	  
	  PinSet::Iterator drvr_iter(drvrs);
	  if (drvr_iter.hasNext()){
	    const Pin* driving_pin = drvr_iter.next();
	    if (unique_set.find(driving_pin) ==
		unique_set.end()){
	      unique_set.insert(driving_pin);
	      cut -> volume_.push_back(cur_inst);
	      cut -> internal_pins_[driving_pin] = lp;	  	      
	      return WalkThroughBuffersAndInverters(cut,driving_pin,unique_set);
	    }
	    else
	      return driving_pin;
	  }
	}
      }
    }
  }
  //default do nothing
  return op_pin;
}

  

bool
  CutGen::EnumerateExpansions( std::vector<const Pin*>& candidate_wavefront,
			       expansionSet& expansion_set){
  //equivalent to the cartesian product, but organized for physical level.

  
    if (candidate_wavefront.size() == 2){
      const Pin* p1 = candidate_wavefront[0];
      const Pin* p2 = candidate_wavefront[1];     
      std::vector<const Pin*> keep;
      std::vector<const Pin*> expand;

      //case analysis
      keep.push_back(p1);
      keep.push_back(p2);
      expansion_set.push_back(std::pair<std::vector<const Pin*>,std::vector<const Pin*> >(keep,expand));

      keep.clear();
      expand.clear();
      keep.push_back(p1);
      expand.push_back(p2);
      expansion_set.push_back(std::pair<std::vector<const Pin*>,std::vector<const Pin*> >(keep,expand));

      keep.clear();
      expand.clear();
      keep.push_back(p2);
      expand.push_back(p1);     
      expansion_set.push_back(std::pair<std::vector<const Pin*>,std::vector<const Pin*> >(keep,expand));

      keep.clear();
      expand.clear();

      expand.push_back(p1);     
      expand.push_back(p2);
      expansion_set.push_back(std::pair<std::vector<const Pin*>,std::vector<const Pin*> >(keep,expand));

      return true;
    }

    //
    //N == 3
    //
    //K:{p1,p2,p3} E:{}
    //K:{p1} E:{p2,p3}
    //K:{p1,p2}  E: {p3}
    //K: {p1,p3} E: {p2}

    //E:{p1,p2,p3} K:{}
    //E:{p1} K:{p2,p3}
    //E:{p1,p2}  K: {p3}
    //E: {p1,p3} K: {p2}
   
    else if (candidate_wavefront.size() == 3){
      const Pin* p1 = candidate_wavefront[0];
      const Pin* p2 = candidate_wavefront[1];
      const Pin* p3 = candidate_wavefront[2];          
      std::vector<const Pin*> keep;
      std::vector<const Pin*> expand;

      //case analysis
      keep.push_back(p1);
      keep.push_back(p2);
      keep.push_back(p3);     
      expansion_set.push_back(std::pair<std::vector<const Pin*>,std::vector<const Pin*> >(keep,expand));
     
      keep.clear();
      expand.clear();

      keep.push_back(p1);
      expand.push_back(p2);
      expand.push_back(p3);     
      expansion_set.push_back(std::pair<std::vector<const Pin*>,std::vector<const Pin*> >(keep,expand));

      keep.clear();
      expand.clear();
      keep.push_back(p1);
      keep.push_back(p2);     
      expand.push_back(p3);     
      expansion_set.push_back(std::pair<std::vector<const Pin*>,std::vector<const Pin*> >(keep,expand));

      keep.clear();
      expand.clear();

      keep.push_back(p1);     
      keep.push_back(p3);
      expand.push_back(p2);     
      expansion_set.push_back(std::pair<std::vector<const Pin*>,std::vector<const Pin*> >(keep,expand));


      keep.clear();
      expand.clear();
      expand.push_back(p1);     
      expand.push_back(p2);
      expand.push_back(p3);     
      expansion_set.push_back(std::pair<std::vector<const Pin*>,std::vector<const Pin*> >(keep,expand));

      keep.clear();
      expand.clear();
      expand.push_back(p1);     
      keep.push_back(p2);
      keep.push_back(p3);     
      expansion_set.push_back(std::pair<std::vector<const Pin*>,std::vector<const Pin*> >(keep,expand));

      keep.clear();
      expand.clear();
      expand.push_back(p1);
      expand.push_back(p2);          
      keep.push_back(p3);     
      expansion_set.push_back(std::pair<std::vector<const Pin*>,std::vector<const Pin*> >(keep,expand));

      keep.clear();
      expand.clear();
      expand.push_back(p1);
      expand.push_back(p3);          
      keep.push_back(p2);     
      expansion_set.push_back(std::pair<std::vector<const Pin*>,std::vector<const Pin*> >(keep,expand));
      return true;
    }
    else if (candidate_wavefront.size() == 4){
      //
      //N == 4
      //
      //K:{p1,p2,p3,p4} E:{}
      
      //K:{p1} E:{p2,p3,p4}
      //K:{p1,p2}  E: {p3,p4}
      //K:{p1,p2,p3} E: {p4}
      
      //K:{p2} E: {p1,p3,p4}
      //K:{p2,p3} E: {p1,p4}
      //K:{p2,p3,p4} E: {p1}

      //K:{p3} E: {p1,p2,p4}
      //K:{p3,p1} E: {p2,p4}
      //K:{p3,p1,p2} E: {p4}

      //K:{} E{p1,p2,p3,p4}
      
    //E:{p1,p2,p3} K:{}
    //E:{p1} K:{p2,p3}
    //E:{p1,p2}  K: {p3}
    //E: {p1,p3} K: {p2}
   
    }

    else if (candidate_wavefront.size() == 5){

    }

    
    return false;
  }

  bool
  CutGen::Boundary(const Pin* cur_pin){
    //a terminal
    if (network_ -> term(cur_pin))
      return true;
    //a memory
    Instance* cur_inst = network_ -> instance(cur_pin);
    if (cur_inst){
      LibertyCell* cell = network_ -> libertyCell(cur_inst);
      if (cell -> isMemory() ||
	  cell -> hasSequentials())
	return true;
    }
    return false;
  }


}
