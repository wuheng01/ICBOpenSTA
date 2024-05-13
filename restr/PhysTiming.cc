/*
Physical remapping: timing interface
------------------------------------

Key idea:
The abc mapper has no idea about slack times on start up.
It uses arrival times on the inputs and required times on the outputs
and then finds the best cover to minimize the slack (ie best timing from
inputs using dynamic programming).

In this code we are extracting the arrival times for each input to a cut
and the required time for each output.

A cut is a cluster of logic gates. It has a root set (the outputs of the cut) 
and a leaf set (the inputs to the cut).

*/
			     
#include <Sdc.hh>
#include <Sta.hh>
#include <Cut.hh>
#include <PhysTiming.hh>
#include <vector>
#include <map>
#include <Network.hh>
#include <Graph.hh>
#include <PathRef.hh>
#include <Corner.hh>

//#define DEBUG_PHYS_TIMING 1

namespace sta {
  void PhysTiming::AnnotateCutTiming(Cut* cut,
				     std::vector<std::pair < const Pin*,TimingRecordP> >
				     &timing_requirements){
    Sta *sta = Sta::sta();
    NetworkReader *network = sta->networkReader();
    Arrival max_arrival_rise = 0.0;
    Arrival max_arrival_fall = 0.0;	
    Required min_required_rise;
    Required min_required_fall;
	
    //Get the arrival and required times for the exceptions for each vertex.
    Sdc *sdc_nwk = sta -> sdc();
    ClockSeq* clock_seq = sdc_nwk -> clocks();
    Graph* graph = sta -> graph();
    
    int ip_ix=0;
    TimingRecordP tr = nullptr;

    //
    //cut inputs: allow for delayed arrival time
    //
    for (auto leaf_pin: cut -> leaves_){
      VertexId v_id = network -> vertexId(leaf_pin);
      Vertex* v = graph -> vertex(v_id);
      tr = nullptr;
      if (v){
	MinMax* min_max_arrival;
	RiseFall* rf_arrival_rise = RiseFall::find("rise");
	RiseFall* rf_arrival_fall = RiseFall::find("fall");

	for (auto clk: *clock_seq){
	  ClockEdge* clk_edge_fall = clk -> edge(rf_arrival_fall);
	  ClockEdge* clk_edge_rise = clk -> edge(rf_arrival_rise);
	  for (auto path_ap : sta->corners()->pathAnalysisPts()) {
	    Arrival v_arrival_rise  =  sta -> vertexArrival(v,rf_arrival_rise, clk_edge_rise, path_ap);
	    Arrival v_arrival_fall = sta -> vertexArrival(v,rf_arrival_fall, clk_edge_fall, path_ap);
	    if (v_arrival_rise > max_arrival_rise)
	      max_arrival_rise = v_arrival_rise;
	    if (v_arrival_fall > max_arrival_fall)
	      max_arrival_fall = v_arrival_fall;
	  }		 
	}
      }
      TimingRecordP tr = nullptr;
      if (graph && v){			    
	tr = new TimingRecord();
	tr -> arrival_rise = max_arrival_rise;
	tr -> arrival_fall = max_arrival_fall;
      }
      timing_requirements.push_back(std::pair<const Pin*,TimingRecordP>(leaf_pin,tr));
      ip_ix++;
#ifdef DEBUG_PHYS_TIMING      
      printf("Pin %s Setting arrival rise %s\n",
		   network -> pathName(leaf_pin),
	     delayAsString(max_arrival_rise,
			   sta
			   ));
      printf("Pin %s Setting arrival fall %s\n",
	     network -> pathName(leaf_pin),
	     delayAsString(max_arrival_fall,
			   sta
			   ));
#endif      
    }

    //cut outputs: set required time
    for (auto root_pin: cut -> root_){
      VertexId v_id = network -> vertexId(root_pin);
      Vertex* v = graph -> vertex(v_id);
      bool got_required=false;
      
      if (v){
	MinMax* min_max_arrival;
	RiseFall* rf_required_rise = RiseFall::find("rise");
	RiseFall* rf_required_fall = RiseFall::find("rise");
	MinMax* min_max = MinMax::find("max");
#ifdef DEBUG_PHYS_TIMING            	
	printf("Root Pin required %s delay: %s\n",
	       network -> pathName(root_pin),
	       delayAsString(sta -> vertexArrival(v,min_max),sta));
#endif	
	for (auto clk: *clock_seq){
	  ClockEdge* clk_edge_fall = clk -> edge(rf_required_fall);
	  ClockEdge* clk_edge_rise = clk -> edge(rf_required_rise);
	  for (auto path_ap : sta->corners()->pathAnalysisPts()) {
	    Required v_required_rise  =  sta -> vertexRequired(v,rf_required_rise, clk_edge_rise, path_ap);
	    Required v_required_fall = sta -> vertexRequired(v,rf_required_fall, clk_edge_fall, path_ap);
	    //default to first required time found.
	    if (!got_required){
	      min_required_rise = v_required_rise;
	      min_required_fall = v_required_fall;
	      got_required= true;
	    }
	    else{
	      if (v_required_rise < min_required_rise)
		min_required_rise = v_required_rise;
	      if (v_required_fall < min_required_fall)
		min_required_fall = v_required_fall;
	    }
	  }		 
	}
      }
      TimingRecordP tr = nullptr;
      if (graph && v && got_required) {
	tr = new TimingRecord();
	tr -> required_rise = min_required_rise;
	tr -> required_fall = min_required_fall;
      }
      timing_requirements.push_back(std::pair<const Pin*,TimingRecordP>(root_pin,tr));

#ifdef DEBUG_PHYS_TIMING            
      printf("Pin %s required rise %s\n",
	     network -> pathName(root_pin),
	     delayAsString(min_required_rise,
			   sta
			   ));
      printf("Pin %s required fall %s\n",
	     network -> pathName(root_pin),
	     delayAsString(min_required_fall,
			   sta
			   ));
#endif      
    }
  }
  
}
