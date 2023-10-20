/*
  Ntk2Cut
  -------
  Build a realization of the abc network in the sta, wire it in
*/

#include "Liberty.hh"
#include "PortDirection.hh"
#include "Network.hh"
#include "ConcreteNetwork.hh"
#include <Cut.hh>
#include <CutGen.hh>
#include <FuncExpr2Kit.hh>
#include <Kit.hh>
#include <Ntk2Cut.hh>

#include <base/abc/abc.h>
#include <base/main/main.h>
#include <base/main/mainInt.h>
#include <base/io/ioAbc.h>
#include "map/mio/mio.h"
#include "map/mio/mioInt.h"

namespace sta {

  using namespace abc;
  
  Ntk2Cut::Ntk2Cut(Abc_Ntk_t* abc_nwk,
		   Cut* cut,
		   ConcreteNetwork* sta_nwk):mapped_netlist_(abc_nwk),
					 cut_(cut),
					 target_nwk_(sta_nwk)
  {
  }

  /*
    Make the sta network for the elements of the cut
  */
  
  void Ntk2Cut::BuildNwkElements(){
    std::map<Abc_Obj_t*,Net*> abc2net;
    std::map<Net*,Abc_Obj_t_*>net2abc;
    std::map<Abc_Obj_t*, Instance*> abc2inst;
    std::map<Instance*, Abc_Obj_t*> inst2abc;
    std::map<const Pin*, Net*> cut_ip_nets;
    
    Abc_Obj_t* pObj=nullptr;
    int i;
    int net_id=0;

    Instance* parent = target_nwk_ -> topInstance();
    //Create a net for each primary input
    for (i=0; i < Abc_NtkPiNum(mapped_netlist_); i++){
      pObj = Abc_NtkPi(mapped_netlist_,i);
      //  std::string ip_net_name = "pi_" + std::to_string(net_id);
      //      Net* ip_net = target_nwk_ -> makeNet(ip_net_name.c_str(),parent);
      //we truly want to change the cut leaf pins here
      Pin* cut_ip_pin = const_cast<Pin*> (cut_-> leaves_[i]);
      Net* ip_net = target_nwk_ -> net(cut_ip_pin);
      if (!ip_net){
	ip_net = target_nwk_ -> findNet(target_nwk_ -> pathName(cut_ip_pin));
	if (!ip_net)
	  printf("Error cannot find ip net for leaf %s\n",
		 target_nwk_ -> pathName(cut_ip_pin)
	       );
      }
      cut_ip_nets[cut_ip_pin]=ip_net;
      abc2net[pObj]=ip_net;
      net2abc[ip_net]=pObj;
      net_id++;
    }
    
    //Create the instances and make a net for each instance output (single output
    //objects only)
    i=0;
    Abc_NtkForEachNode(mapped_netlist_, pObj,i){
      if (pObj -> Type == ABC_OBJ_PI ||
	  pObj -> Type == ABC_OBJ_PO){
	continue;
      }
      else{
	Mio_Gate_t* mapped_gate = (Mio_Gate_t*)(pObj -> pData);
	std::string op_port_name = mapped_gate -> pOutName;
	char* lib_gate_name = Mio_GateReadName(mapped_gate);
	LibertyCell* liberty_cell = target_nwk_ -> findLibertyCell(lib_gate_name);
	Instance* cur_inst = target_nwk_ -> makeInstance(liberty_cell, lib_gate_name,parent);
	target_nwk_ -> makePins(cur_inst);
	LibertyPort* liberty_op_port = liberty_cell -> findLibertyPort(op_port_name.c_str());
	
	inst2abc[cur_inst] = pObj;
	abc2inst[pObj]=cur_inst;
      
	std::string inst_name(lib_gate_name);
	std::string op_net_name = inst_name + "_" + std::to_string(net_id);
	net_id++;
	Net* op_net = target_nwk_ -> makeNet(op_net_name.c_str(),parent);
	abc2net[pObj] = op_net;
	net2abc[op_net] = pObj;
	target_nwk_ -> connect(cur_inst,liberty_op_port,op_net);
      }
    }

    i=0;
    Abc_NtkForEachNode(mapped_netlist_, pObj,i){
      if (pObj -> Type == ABC_OBJ_PI ||
	  pObj -> Type == ABC_OBJ_PO){
	continue;
      }
      else{
	//Wire up the inputs of the instances	  
	Mio_Gate_t* mapped_gate = (Mio_Gate_t*)(pObj -> pData);
	char* lib_gate_name = Mio_GateReadName(mapped_gate);
	LibertyCell* liberty_cell = target_nwk_ -> findLibertyCell(lib_gate_name);
	Instance* cur_inst = abc2inst[pObj];
	//get the inputs
	int fanin_count = Abc_ObjFaninNum(pObj);
	for (int ip_ix = 0; ip_ix < fanin_count; ip_ix++){
	  Abc_Obj_t* driver = Abc_ObjFanin(pObj,ip_ix);
	  Net* driving_net = abc2net[driver];
	  int count = 0;
	  while (!driving_net && count < 10){
	    driver = Abc_ObjFanin(driver,0);
	    if (!driver){
	      printf("Cannot find fanin driver for abc object %d\n", pObj);
	    }
	    driving_net = abc2net[driver];
	    if (!driving_net){
	      printf("Cannot find net for abc obj %d\n",driver ->Id);
	    }
	    count++;
	  }
	  if (!driving_net){
	    printf("Cannot find input %d driver for abc obj %d of type %d which isdriven by abc obj %d\n",
		   ip_ix,
		   pObj -> Id,
		   pObj -> Type,
		   driver -> Id);
	    continue;
	  }
	  assert(driving_net);
	  char* dest_name = Mio_GateReadPinName(mapped_gate,ip_ix);
	  Pin* dest_pin = target_nwk_ -> findPin(cur_inst,dest_name);
	  //connect driving pin to this pin
	  if (!target_nwk_ -> isConnected(driving_net,dest_pin))
	    target_nwk_ -> connectPin(dest_pin,driving_net);
	}
      }
    }
    //Wire up the outputs
    for (i=0; i < Abc_NtkPoNum(mapped_netlist_); i++){
      pObj = Abc_NtkPo(mapped_netlist_,i);
      //get the fanin driver
      //demote this assert to a test
      assert(Abc_ObjFaninNum(pObj)>0);
      Abc_Obj_t_* fanin = Abc_ObjFanin(pObj,0);
      Net* driving_net = abc2net[fanin];
      if (!driving_net){
	fanin = Abc_ObjFanin(fanin,0);
	driving_net = abc2net[fanin];
      }
      //we are intentionally modifying this pin, so unconst it
      Pin* cut_op_pin = const_cast<Pin*>(cut_-> root_[i]);
      if (!(target_nwk_ -> isConnected(driving_net,cut_op_pin)))
	target_nwk_ -> connectPin(cut_op_pin,driving_net);
    }
    //STA will clean up the connections as we delete the instances
    for (auto cur_inst : cut_ -> volume_){
      target_nwk_ -> deleteInstance(const_cast<Instance*>(cur_inst));
    }
  }
}

