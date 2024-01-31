#include <Sta.hh>
#include "Liberty.hh"
#include "PortDirection.hh"
#include "Network.hh"
#include <Cut.hh>
#include <CutGen.hh>
#include <FuncExpr2Kit.hh>
#include <Kit.hh>
#include <Cut2Ntk.hh>
#include <Graph.hh>
#include "ClusterCutGen.hh"
#include <VertexVisitor.hh>

namespace sta {

  class AccumulateSortedSlackEndPoints: public VertexVisitor {
    
  public:
    AccumulateSortedSlackEndPoints(std::map<float,Vertex*>&
				   sorted_slack_times,
				   std::map<Vertex*,float>&
				   vertex_slack_times,
				   std::set<Vertex*>&   end_points
				   ):sorted_slack_times_(sorted_slack_times),
				     vertex_slack_times_(vertex_slack_times),
				     end_points_(end_points)
    {}
    void visit(Vertex* visit);
    VertexVisitor* copy() const{return nullptr;}
  private:
    std::map<float,Vertex*> &sorted_slack_times_;
    std::map<Vertex*,float> &vertex_slack_times_;
    std::set<Vertex*> end_points_;
    friend class ClusterCutGen;
  };

  void
  AccumulateSortedSlackEndPoints::visit(Vertex* v){
    Sta* sta = Sta::sta();
    Slack s = sta -> vertexSlack(v, MinMax::max());
    sorted_slack_times_[s]=v;
    vertex_slack_times_[v]=s;
    end_points_.insert(v);
  }


  /*
    Kernel algorithm for cluster cut generation:
    -------------------------------------------
    1. queue = Set up end points sorted by criticallity
    2. Pick most critical end point
    3. Walk back to invariant drivers. Set up cut Leaves
    4. Walk forwards to end points. Set up cut roots.
    5. Extract cut and add to cut set.
    6. Remove cut roots from queue.
    7. If queue not empty go to step 1.
    8. Return cut set.
  */
  void
  ClusterCutGen::GenClusters(){
    Sta* sta = Sta::sta();    
    std::map<float,Vertex*> sorted_slack_times; //slack -> vertex
    std::map<Vertex*,float> vertex_slack_times; //vertex -> slack

    int cut_id=0;
    
    //visitor
    AccumulateSortedSlackEndPoints accumulate_sorted_slack_end_points(
								      sorted_slack_times,
								      vertex_slack_times,
								      end_points_);
    //accumulate
    sta -> visitEndpoints(&accumulate_sorted_slack_end_points);
    
    while (vertex_slack_times.size() != 0){
      //map stores smallest item first, so this is by default the least slack
      Vertex* head_vertex = (*sorted_slack_times.begin()).second;
      Pin* cur_pin = head_vertex -> pin();
      ResetCutTemporaries();
      pin_visited_.insert(cur_pin);
      WalkBackwardsToTimingEndPointsR(cur_pin,0);
      
      for (auto leaf_pin_int: leaves_){
	WalkForwardsToTimingEndPointsR(leaf_pin_int.first,0);
      }
      Cut* cut = ExtractCut(cut_id);
      cutset_.push_back(cut);
      //remove visited vertices from stash of sorted & vertex slack times
      for (auto cut_root: cut -> root_){
	Vertex* root_vertex = graph_ -> vertex(nwk_ -> vertexId(cut_root));
	UpdateVertexList(root_vertex,
			 sorted_slack_times,
			 vertex_slack_times);
      }
      cut_id++;
    }
  }

  
  Cut*
  ClusterCutGen::ExtractCut(int cut_id){

    Cut* ret = new Cut();
    ret -> leaves_.resize(leaves_.size());
    for (auto i: leaves_)
      ret -> leaves_[i.second]=i.first;
    
    ret -> root_.resize(roots_.size());
    for (auto i: roots_)
      ret -> root_[i.second]=i.first;
    
    for (auto i: cut_volume_)
      ret -> volume_.push_back(i);
    return ret;
  }
  

  void ClusterCutGen::UpdateVertexList(Vertex* root,
				       std::map<float,Vertex*>& float2vertex,
				       std::map<Vertex*,float>& vertex2float){
    std::map<float,Vertex*>::iterator it1 = float2vertex.begin();
    Vertex* cur_vertex = (*it1).second;
    float2vertex.erase(it1);
    std::map<Vertex*,float>::iterator it2 = vertex2float.find(cur_vertex);
    vertex2float.erase(it2);
  }

  

  
  void
  ClusterCutGen::WalkForwardsToTimingEndPointsR(const Pin* start_pin, int depth){
    if (start_pin && pin_visited_.find(start_pin) == pin_visited_.end()){
      pin_visited_.insert(start_pin);
      Port* start_port = nwk_ -> port(start_pin);
      Vertex* vertex = graph_ -> vertex(nwk_ -> vertexId(start_pin));
      
      if ((end_points_.find(vertex)!= end_points_.end()) &&
	  depth !=0){
	roots_[start_pin]=roots_.size();
	return;
      }
      if (nwk_ -> direction(start_port) == PortDirection::output()){
	//traverse forwards
	PinConnectedPinIterator *connected_pin_iter = nwk_ -> connectedPinIterator(start_pin);
	while (connected_pin_iter -> hasNext()){
	  const Pin* connected_pin = connected_pin_iter -> next();
	  WalkForwardsToTimingEndPointsR(connected_pin,depth+1);
	}
      }
      else{
	Instance* cur_inst = nwk_ -> instance(start_pin);
	cut_volume_.insert(cur_inst);
	InstancePinIterator* pi = nwk_ -> pinIterator(cur_inst);
	while (pi -> hasNext()){
	  Pin* cur_pin = pi -> next();
	  if (nwk_ -> direction(cur_pin) == PortDirection::output()){
	    WalkForwardsToTimingEndPointsR(cur_pin,depth+1);
	  }
	}
      }
    }
  }
  
  void
  ClusterCutGen::WalkBackwardsToTimingEndPointsR(const Pin* start_pin, int depth){
    if (start_pin && pin_visited_.find(start_pin) == pin_visited_.end()){
      pin_visited_.insert(start_pin);
      Port* start_port = nwk_ -> port(start_pin);
      Vertex* vertex = graph_ -> vertex(nwk_ -> vertexId(start_pin));

      //hit an invariant end point
      if ((end_points_.find(vertex) != end_points_.end()) && depth != 0){
	leaves_[start_pin]=leaves_.size();
	return;
      }
      
      if (nwk_ -> direction(start_port) == PortDirection::input()){
	//traverse back
	PinSet* drivers = nwk_ -> drivers(start_pin);
	if (drivers){
	  PinSet::Iterator drvr_iter(drivers);
	  const Pin* driving_pin = drvr_iter.next();
	  WalkBackwardsToTimingEndPointsR(driving_pin, depth+1);
	}
      }
      else{
	//push up to parent and walk back through input pins
	Instance* cur_inst = nwk_ -> instance(start_pin);
	cut_volume_.insert(cur_inst);	
	InstancePinIterator* pi = nwk_ -> pinIterator(cur_inst);
	while (pi -> hasNext()){
	  Pin* cur_pin = pi -> next();
	  if (nwk_ -> direction(cur_pin) == PortDirection::input()){
	    WalkBackwardsToTimingEndPointsR(cur_pin,depth+1);
	  }
	}
      }
    }
  }
  

}
