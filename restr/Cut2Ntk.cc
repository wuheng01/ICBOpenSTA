/*
Cut2Ntk
-------
Build an abc netlist for a cut.
*/

#include "Liberty.hh"
#include "PortDirection.hh"
#include "Network.hh"
#include <Cut.hh>
#include <CutGen.hh>
#include <FuncExpr2Kit.hh>
#include <Kit.hh>
#include <Cut2Ntk.hh>


#include <base/abc/abc.h>
#include <base/main/main.h>
#include <base/main/mainInt.h>
#include <base/io/ioAbc.h>

//#define CUT2NWK_DEBUG 1

namespace sta {

  bool Cut2Ntk::SopNwkCheck(Abc_Ntk_t* abc_nwk){

    int i;
    Abc_Obj_t* pObj;
    //force to logic
    Abc_Ntk_t_* abc_nwk_in ;
    if (!Abc_NtkIsLogic)
      abc_nwk_in = Abc_NtkToLogic(abc_nwk);
    else
      abc_nwk_in = abc_nwk;
    printf("Nwk %s\n", abc_nwk_in -> pName);
    printf("PO Objects\n");
    Abc_NtkForEachObj(abc_nwk_in, pObj,i){
      if (pObj -> Type == ABC_OBJ_PO){
	printf("Primary Op Abc Obj %d\n",pObj -> Id);
	int j;
	Abc_Obj_t* pFanin;
	Abc_ObjForEachFanin(pObj,pFanin,j){
	  printf(" <- %d\n", pFanin -> Id);
	}
      }
    }

    printf("PI Objects\n");
    Abc_NtkForEachObj(abc_nwk_in, pObj,i){
      if (pObj -> Type == ABC_OBJ_PI){
	printf("Primary Ip Abc Obj %d\n",pObj -> Id);
      }
    }

    printf("Nets\n");
    Abc_NtkForEachObj(abc_nwk_in, pObj,i){
      if (!(pObj -> Type == ABC_OBJ_PI ||
	    pObj -> Type == ABC_OBJ_PO)){
	if (!pObj -> pData) {
	printf("Net: Abc Obj %d fn: %s\n",pObj -> Id, pObj -> pData ? (char*)(pObj -> pData):"");
	Abc_Obj_t* pFanin;
	int j=0;
	Abc_ObjForEachFanin(pObj,pFanin,j){
	  printf(" <- %d ", pFanin -> Id);
	  if (pFanin -> pData == nullptr &&
	      pFanin -> Type != ABC_OBJ_PI)
	    printf("Bad net: net -> net connection ");
	  printf("\n");
	  
	}
	j=0;
	Abc_Obj_t* pFanout;
	Abc_ObjForEachFanout(pObj,pFanout,j){
	  printf(" -> %d ", pFanout -> Id);
	  if (pFanout -> pData == nullptr &&
	      pFanout -> Type != ABC_OBJ_PO)
	    printf("Bad net : net -> net connection ");
	  printf("\n");
	}
	}
      }
    }
    
    printf("Generic Objects\n");
    Abc_NtkForEachObj(abc_nwk_in, pObj,i){
      if (!(pObj -> Type == ABC_OBJ_PI ||
	    pObj -> Type == ABC_OBJ_PO)){
	if (pObj -> pData) {
	printf("Abc Obj %d fn: %s\n",pObj -> Id, pObj -> pData ? (char*)(pObj -> pData):"");
	Abc_Obj_t* pFanin;
	int j;
	Abc_ObjForEachFanin(pObj,pFanin,j){
	  printf(" <- %d\n", pFanin -> Id);
	}
	}
      }
    }

    printf("CO\n");
    Abc_NtkForEachCo(abc_nwk_in,pObj,i){
      printf("Co %d\n",pObj -> Id);
      printf("Num drivers %d\n",Abc_ObjFaninNum(pObj));
      Abc_Obj_t* pFanin;
      int j;
      Abc_ObjForEachFanin(pObj,pFanin,j){
	printf(" <- %d\n", pFanin -> Id);
      }
    }
    printf("CI\n");
    Abc_NtkForEachCi(abc_nwk_in,pObj,i){
      printf("Ci %d\n",pObj -> Id);
    }
    if (!Abc_NtkIsNetlist(abc_nwk_in)){
      Abc_Ntk_t* nwk_nl = Abc_NtkToNetlist(abc_nwk_in);
      Abc_NtkSetName(nwk_nl,Extra_UtilStrsav("HappyCat"));
      Io_WriteBlif(nwk_nl,"debug.blif",0,0,0);
    }
    else{
      Abc_NtkSetName(abc_nwk_in,Extra_UtilStrsav("HappyCat"));
      Io_WriteBlif(abc_nwk_in,"debug.blif",0,0,0);
    }
    
    if (!Abc_NtkIsSopNetlist(abc_nwk_in)){
      Abc_Ntk_t* sop_netlist = Abc_NtkToNetlist(abc_nwk_in);
      Abc_NtkSetName(sop_netlist,Extra_UtilStrsav(abc_nwk -> pName));
      Abc_NtkToAig(sop_netlist);      
      Io_WriteBenchLut(sop_netlist,"debug.bench");
    }
    else{
      
      Abc_NtkSetName(abc_nwk_in,Extra_UtilStrsav("DebugCat"));
      Abc_Ntk_t* sop_logic_nwk = Abc_NtkToLogic(abc_nwk_in);
      //      Abc_Ntk_t* strashed_nwk = Abc_NtkStrash(sop_logic_nwk,0,0,0);
      Abc_Ntk_t* nwk_temp = Abc_NtkToNetlist(sop_logic_nwk);
      Abc_NtkToAig(nwk_temp);
      if (Abc_NtkIsAigNetlist(nwk_temp))
	printf("An aig netlist\n");
      if (Abc_NtkIsSopNetlist(nwk_temp))
	printf("An sop netlist\n");
      Io_WriteBenchLut(nwk_temp,"debug.bench");      
    }
    return true;
  }

  /*
    To keep our connection algorithm simple we make lots of nets
    (eg on the inputs and outputs of instances as well as pi/po.
    This means we can get net -> net connections, which abc accepts
    but does not like. So this routine iteratively removes the
    net -> net connections until all are gone.
  */
  
  void Cut2Ntk::CleanUpNetNets(){
    bool done_something = true;
    
    while (done_something){
      done_something = false;
      std::vector<Abc_Obj_t*> orphans;
      for (auto pin_net: net_table_abc2sta_){
      Abc_Obj_t* cur_net = pin_net.first;

      //paranoia just in case we have changed this net already.
      if (!cur_net)
	continue;
      if (Abc_ObjFaninVec(cur_net) -> nSize == 0)
	continue;
      
      Abc_Obj_t* fanin_obj = Abc_ObjFanin0(cur_net);
      if (net_table_abc2sta_.find(fanin_obj)!=
	  net_table_abc2sta_.end()){
	Abc_Obj_t* fanin_fanin_obj = Abc_ObjFanin0(fanin_obj);
	
#ifdef CUT2NWK_DEBUG	
	printf("Cleaning up net -> net: %d -> %d\n",
	       fanin_obj -> Id,
	       cur_net -> Id);
#endif
	//delete the cur net
	//fanin_fanin_obj -> fanin_obj -> cur_net -> {FO}
	//-->
	//fanin_fanin_obj -> fanin_obj -> {FO}
	//
	Abc_ObjDeleteFanin(cur_net,fanin_obj);
	Abc_ObjTransferFanout(cur_net,fanin_obj);
	//at this point cur_net is totally disconnected
	orphans.push_back(cur_net);
	done_something = true;
      }
    }
    //kill the orphans
      for (auto n: orphans){
	const Pin* cur_pin = net_table_abc2sta_[n];
	net_table_abc2sta_.erase(n);
	net_table_sta2abc_.erase(cur_pin);
	Abc_NtkDeleteObj(n);
      }
    }
  }
  
  
  Abc_Ntk_t* Cut2Ntk::BuildSopLogicNetlist(){
    
#ifdef CUT2NWK_DEBUG	
    printf("working with cut :\n");
    cut_ -> Print(stdout, sta_nwk_);
#endif    
    abc_nwk_ = (Abc_NtkAlloc(ABC_NTK_NETLIST, ABC_FUNC_SOP,1));
    
    Abc_NtkSetName(abc_nwk_,Extra_UtilStrsav(cut_name_.c_str()));
    //create the Primary i/o corresponding to the terminals of the cut
    for (auto po: cut_ -> root_){
      Abc_Obj_t* abc_po = Abc_NtkCreatePo(abc_nwk_);
      po_table_[po] = abc_po;
      Abc_ObjAssignName(abc_po, (char*)(sta_nwk_ -> pathName(po)),NULL);
      Abc_Obj_t* abc_op_net = Abc_NtkCreateNet(abc_nwk_);
      Abc_ObjAssignName(abc_op_net, (char*)(sta_nwk_ -> pathName(po)),NULL);      
      Abc_ObjAddFanin(abc_po,abc_op_net);
#ifdef CUT2NWK_DEBUG	      
      printf("Po %s abc: %d\n", (char*)(sta_nwk_ -> pathName(po)),abc_po -> Id);
      printf("Root pin id %d\n",abc_po -> Id);
#endif      
      net_table_sta2abc_[po]=abc_op_net;
      net_table_abc2sta_[abc_op_net]=po;
    }
    
    for (auto pi: cut_ -> leaves_){
      Abc_Obj_t* abc_pi = Abc_NtkCreatePi(abc_nwk_);
      Abc_ObjAssignName(abc_pi,(char*)(sta_nwk_ -> pathName(pi)),NULL);
      pi_table_[pi] = abc_pi;
      Abc_Obj_t* abc_ip_net = Abc_NtkCreateNet(abc_nwk_);
      Abc_ObjAssignName(abc_ip_net,(char*)(sta_nwk_ -> pathName(pi)),NULL);      
      Abc_ObjAddFanin(abc_ip_net,abc_pi);
#ifdef CUT2NWK_DEBUG	      
      printf("Adding fanin %d to %d\n",
	     abc_ip_net -> Id,
	     abc_pi -> Id);
#endif      
      net_table_sta2abc_[pi]=abc_ip_net;
      net_table_abc2sta_[abc_ip_net] = pi;
    }
    
    //create the instances for each gate in the cut
    for (auto cur_inst: cut_ -> volume_){
      Abc_Obj_t* cur_gate = CreateGate(cur_inst);
#ifdef CUT2NWK_DEBUG	      
      printf("Created Gate %d with %d fanin\n",
	     cur_gate -> Id,
	     cur_gate -> vFanins.nSize);
#endif      
    }
    
    //Wire Up all the inputs for each gate
    for (auto cur_inst: cut_ -> volume_){
#ifdef CUT2NWK_DEBUG	      
      printf("Wiring up instance %s\n", sta_nwk_ -> pathName(cur_inst));
#endif      
      InstancePinIterator *pin_iter = sta_nwk_ -> pinIterator(cur_inst);
      while (pin_iter -> hasNext()){
	const Pin* ip_pin = pin_iter-> next();
	if (sta_nwk_ -> direction (ip_pin) -> isInput()){
	  Abc_Obj_t* ip_net = net_table_sta2abc_[ip_pin];
	  PinSet* drvrs = sta_nwk_ -> drivers(ip_pin);
	  if (drvrs){
	    PinSet::Iterator drvr_iter(drvrs);
	    const Pin* driving_pin = drvr_iter.next();
	    Abc_Obj_t* driver = net_table_sta2abc_[driving_pin];
	    //in case when ip is driven by PI we have already
	    //stashed that pi driver with the input pin
	    //so the ip_net and driver will be the same
	    //so skip that case.
	    if (ip_net != driver){
#ifdef CUT2NWK_DEBUG		      
	      printf("Driving pin %s\n",
		     sta_nwk_ -> pathName(driving_pin));
	      printf("instance ip net Adding fanin driver %d to %d\n",
		     driver -> Id,
		     ip_net -> Id		     
		     );
#endif	      
	      Abc_ObjAddFanin(ip_net,driver);
	    }
	  }
	}
      }
    }
#ifdef CUT2NWK_DEBUG	    
    printf("Wiring up the roots\n");
#endif    
    //Wire Up all the primary outputs (roots);
    //We may have wired up the root when creating the gates
    //in which case the abc object for the root will have
    //a fanin count > 0.
    
    for (auto root_pin : cut_ -> root_){
#ifdef CUT2NWK_DEBUG    	        
      printf("Root pin %s\n",sta_nwk_ -> pathName(root_pin));
#endif      
      Abc_Obj_t* abc_sink_net = net_table_sta2abc_[root_pin];
#ifdef CUT2NWK_DEBUG    	              
      printf("Root pin id %d\n", abc_sink_net -> Id);
#endif
      
      //make sure not already wired up
      if (Abc_ObjFaninNum(abc_sink_net) == 0){
#ifdef CUT2NWK_DEBUG    	  	
	printf("Root net Abc id %d\n",abc_sink_net -> Id);
#endif	
	PinSet* drvrs = sta_nwk_ -> drivers(root_pin);
	if (drvrs){
	  PinSet::Iterator drvr_iter(drvrs);
	  const Pin* driving_pin = drvr_iter.next();
	  Abc_Obj_t* abc_driver = net_table_sta2abc_[driving_pin];
	  assert(abc_driver);
	  Abc_ObjAddFanin(abc_sink_net /* thing driven */,abc_driver /* fanin */);
#ifdef CUT2NWK_DEBUG    	  
	  printf("primary op Adding fanin driver %d to %d\n",
		 abc_driver -> Id,
		 abc_sink_net -> Id);
#endif	  
	}
      }
      else{
#ifdef CUT2NWK_DEBUG    	  	
	printf("Root %d already connected to %d\n",
	       abc_sink_net -> Id,
	       Abc_ObjFanin0(abc_sink_net) -> Id);
#endif
	;
      }
    }

    //remove the net -> net connections
    CleanUpNetNets();
    
#ifdef CUT2NWK_DEBUG    
    printf("Checking the sop network!\n");
    fflush(stdout);
#endif    

    Abc_NtkFinalizeRead(abc_nwk_);

    Abc_Frame_t* gf = Abc_FrameGetGlobalFrame();
    gf -> pNtkCur = abc_nwk_;
    
    //Check the network using the abc network checker    
    assert(Abc_NtkCheck(abc_nwk_));

    
    //code to inspect the netlist
#ifdef CUT2NWK_DEBUG

    if (Abc_NtkIsSopNetlist(abc_nwk_))
      printf("SOP netlist:\n");
    else
      printf("NON SOP netlist:\n");

    if (Abc_NtkIsLogic(abc_nwk_))
      printf("Logic Netlist\n");
    else
      printf("Non logic netlist\n");

#endif
    Abc_NtkSetName(abc_nwk_,Extra_UtilStrsav(cut_name_.c_str()));		   
    SopNwkCheck(abc_nwk_);



    return Abc_NtkDup(abc_nwk_);
  }

  
  Abc_Obj_t*
  Cut2Ntk::CreateGate(const Instance* cur_inst){

    Abc_Obj_t* ret = nullptr;
    //make the input nets.
    InstancePinIterator *pin_iter = sta_nwk_ -> pinIterator(cur_inst);
    std::vector<Abc_Obj_t*> ip_nets;    
    while (pin_iter -> hasNext()){
      const Pin* cur_pin = pin_iter -> next();
      if (sta_nwk_->direction (cur_pin) -> isInput()){
	//Check, if driven by a Primary don't create the net
	PinSet* drvrs = sta_nwk_ -> drivers(cur_pin);
	if (drvrs){
	  PinSet::Iterator drvr_iter(drvrs);
	  const Pin* driving_pin = drvr_iter.next();
	  if (net_table_sta2abc_.find(driving_pin) !=
	      net_table_sta2abc_.end()){
	    Abc_Obj_t* driver = net_table_sta2abc_[driving_pin];
	    ip_nets.push_back(driver);
	    net_table_sta2abc_[cur_pin]=driver;
	  }
	  else{
	    Abc_Obj_t* ip_net = Abc_NtkCreateNet(abc_nwk_);
	    Abc_ObjAssignName(ip_net, (char*)(sta_nwk_ -> pathName(cur_pin)),NULL);      	    
	    ip_nets.push_back(ip_net);
	    net_table_sta2abc_[cur_pin]=ip_net;
	    net_table_abc2sta_[ip_net]=cur_pin;
	  }
	}
      }
    }

    pin_iter = sta_nwk_ -> pinIterator(cur_inst);    
    while (pin_iter -> hasNext()){
      const Pin* op_pin = pin_iter -> next();
      if (sta_nwk_->direction (op_pin) -> isOutput()){
	std::vector<Abc_Obj_t*> product_terms;
	LibertyPort* lp = sta_nwk_ -> libertyPort(op_pin);
	if (lp && lp -> function()){
	  //Get the kit representation
	  int var_count = 0;
	  /*word32*/ unsigned*  kit_tables;
	  FuncExpr2Kit(lp-> function(),var_count,kit_tables);
	  unsigned num_bits_to_set = (0x01 << var_count );			  
	  unsigned mask=0x00;
	  for (unsigned j=0; j < num_bits_to_set; j++)
	    mask = mask | (0x1<< j);
	  unsigned kit_result = *kit_tables & mask;
	  

	  printf("Creating gate for %s (%d 0x%x)\n", lp -> function() -> asString(),
		 var_count, kit_result);

	  
	  char* fn = Abc_SopCreateFromTruth(static_cast<Mem_Flex_t_*>(abc_nwk_->pManFunc),
					    var_count,kit_tables);
	  ret = Abc_NtkCreateNode(abc_nwk_);
	  printf("Abc string %s\n",fn);
	  Abc_ObjSetData(ret, fn);

	  //wire up the inputs
	  for (int i=0; i < var_count; i++)
	    Abc_ObjAddFanin(ret,ip_nets[i]);

	  
	  //wire up the output
	  if (net_table_sta2abc_.find(op_pin)!=
	      net_table_sta2abc_.end()){
	    Abc_Obj_t* op_wire = net_table_sta2abc_[op_pin];
	    Abc_ObjAddFanin(op_wire,ret);
#ifdef CUT2NWK_DEBUG	    
	    printf("3 Making connection from %d -> %d\n",
		   ret -> Id,
		   op_wire -> Id);
#endif	    
	  }
	  else{
	    //make net and wire it in
	    Abc_Obj_t* sop_op_net = Abc_NtkCreateNet(abc_nwk_);
	    Abc_ObjAssignName(sop_op_net, (char*)(sta_nwk_ -> pathName(op_pin)),NULL);      	    	    
	    Abc_ObjAddFanin(sop_op_net ,ret);
#ifdef CUT2NWK_DEBUG	    
	    printf("4 Making connection from %d to %d\n", ret -> Id, sop_op_net -> Id);
#endif	    
	    net_table_sta2abc_[op_pin]=sop_op_net;
	    net_table_abc2sta_[sop_op_net]=op_pin;
	  }
	}
      }
    }
    return ret;
  }

    



  
} //sta namespace
