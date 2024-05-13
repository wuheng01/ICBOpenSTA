/*

Physical Remapping: Map a Cut subject to timing constraints.
-----------------------------------------------------------

Core idea:
This code takes a cut of logic (which is a cluster of gates : the inputs to the cluster
are called leaves and the outputs roots), generates the abc data structures for the cut,
annotates timing constraints and then reapplies the abc mapper (amap) using the timing
constraints. Finally it rebuilds the area of logic modified (the cut) in the sta network.
*/



#include <base/abc/abc.h>
#include <vector>
#include <Cut.hh>
#include <PhysTiming.hh>
#include <PhysRemap.hh>
#include <Ntk2Cut.hh>
#include <ConcreteNetwork.hh>
#include <map/amap/amapInt.h>
#include <VerilogWriter.hh>
#include <base/main/main.h>
extern "C" void Io_WriteVerilog(abc::Abc_Ntk_t*, char*, int);

namespace abc {
typedef struct Abc_Frame_t_ Abc_Frame_t;
typedef struct Abc_Ntk_t_   Abc_Ntk_t;
Abc_Frame_t * Abc_FrameGetGlobalFrame();
void Amap_ManSetDefaultParams(Amap_Par_t*);
Abc_Ntk_t* Abc_NtkDarAmap(Abc_Ntk_t*, Amap_Par_t*);
}

using namespace abc;

namespace sta {
  
  void
  PhysRemap::Remap(){
    if (script_){
      printf("Applying script %s\n", script_to_apply_.c_str());
    }
    //Build the abc network for the cut. This is sta -> abc conversion
    Cut2Ntk c2nwk (nwk_,cut_,cut_name_);
    Abc_Ntk_t* sop_logic_nwk = c2nwk.BuildSopLogicNetlist();
    //Annotate the timing requirements
    AnnotateTimingRequirementsOntoABCNwk(sop_logic_nwk);
    //Remap the logic after structure hashing and balancing
    Abc_Ntk_t* cut_logic = Abc_NtkToLogic(sop_logic_nwk);
    if (script_){
      Abc_Ntk_t* pNtk = Abc_NtkStrash(cut_logic,0,0,0);
      Abc_FrameReplaceCurrentNetwork(Abc_FrameGetGlobalFrame(), pNtk);
      int status = 1;
      status = Cmd_CommandExecute(Abc_FrameGetGlobalFrame(),
				  script_to_apply_.data());
      if (status ==0){
	//get the mapped netlist from the global frame
	//make sure the final result is mapped !
	Abc_Ntk_t* mapped_netlist = Abc_FrameReadNtk(Abc_FrameGetGlobalFrame());
	BuildCutRealization(mapped_netlist);
      }
      else{
	printf("Error running script %s\n",script_to_apply_.c_str());
      }
    }
    else{
      Abc_Ntk_t* pNtk = Abc_NtkStrash(cut_logic,0,0,0);
      Abc_Ntk_t* pNtk_res = Abc_NtkBalance(pNtk,0,0,1);
    Amap_Par_t Pars_local;
    Amap_ManSetDefaultParams(&Pars_local);
    //favour timing
    Pars_local.fADratio = 0.9; //favour timing
    Pars_local.fVerbose = 1;
    //invoke amap
    Abc_Ntk_t* mapped = Abc_NtkDarAmap(pNtk_res,&Pars_local);
    Abc_Ntk_t* mapped_netlist = Abc_NtkToNetlist(mapped);

#ifdef PHYSREMAP_DEBUG        
    if (!Abc_NtkHasAig(mapped_netlist) && !Abc_NtkHasMapping(mapped_netlist))
      Abc_NtkToAig(mapped_netlist);
    Io_WriteVerilog(mapped_netlist,"premap_netlist.v",0);
#endif

    //stitch the result back into the physical
    //netlist.
    BuildCutRealization(mapped_netlist);
    }
  }

  
  void
  PhysRemap::AnnotateTimingRequirementsOntoABCNwk(Abc_Ntk_t* sop_logic_nwk){
    //Axiom the order in which the abc pi/po is made is :
    //inputs: leaf order in vector
    //outputs: root order in vector
    //So leaf at [0] corresponds to first abc CI.
    //So element at [leaf_count.size()] is first abc CO.
    int ip_ix =0;
    int op_ix =0;
    int ix =0;
    //timing requirements are set in PhysTiming.cc/.hh
    for (auto tr: timing_requirements_){
      const Pin* sta_pin = tr.first;
      TimingRecordP trec = tr.second;
      //inputs set arrival time
      if (ix < cut_ -> leaves_.size() && trec){
	ip_ix = ix;
	Abc_Obj_t* pi = (Abc_Obj_t*)(Vec_PtrEntry(sop_logic_nwk -> vCis,ip_ix));
	Abc_NtkTimeSetArrival(sop_logic_nwk,pi -> Id, trec -> arrival_rise, trec -> arrival_fall);	      }
      //outputs set required time
      else{
	if (trec){
	  op_ix = ix - cut_ -> leaves_.size();
	  Abc_Obj_t* po = (Abc_Obj_t*)(Vec_PtrEntry(sop_logic_nwk -> vCos,op_ix));
	  Abc_NtkTimeSetRequired(sop_logic_nwk,po -> Id, trec -> arrival_rise, trec -> arrival_fall);	      }
      }
      ix++;
    }
  }

  void
  PhysRemap::BuildCutRealization(Abc_Ntk_t* nwk){
    //Build the cut in the network.
    //And delete the original volume.
    Ntk2Cut n2c (nwk,cut_,dynamic_cast<ConcreteNetwork*>(nwk_));
    n2c.BuildNwkElements();
  }

  
}
  

