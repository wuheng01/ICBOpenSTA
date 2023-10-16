/*
ClusterCutGen:
-------------
Generate a cluster cut. This is a cut comprising
connected nodes contibuting to a failing end 
point.
*/

#pragma once

#include <vector>
#include <Cut.hh>

namespace sta {
  class Network;
  class Pin;
  class Instance;
  class Cut;
  class Graph;
  
  class ClusterCutGen {
    
public:
    ClusterCutGen(Network* nwk):nwk_(nwk){graph_ = Sta::sta() -> graph();}
    void GenClusters();
    
    Vertex* PickMostNegativeSlack(std::map<float,Vertex*>&){return nullptr;}
    
    bool InvariantPoint(Pin*) {return false;}
    void WalkBackwardsToTimingEndPointsR(Pin*,int depth);
    void WalkForwardsToTimingEndPointsR(Pin* start_pin, int depth);
    Cut* ExtractCut(int id);
    void ResetCutTemporaries(){ leaves_.clear(); roots_.clear(); cut_volume_.clear();ordered_leaves_.clear();ordered_roots_.clear(); pin_visited_.clear();}
    void UpdateVertexList(Vertex* root,
			  std::map<float,Vertex*>&,
			  std::map<Vertex*,float>&
			  );
    std::vector<Cut*>& cutset(){return cutset_;}
private:
    Network *nwk_;
    Graph* graph_;
    std::set<Vertex*> end_points_; //full set of end points
    std::vector<Cut*> cutset_;

    //temporaries for cut building
    std::map<Pin*,int> leaves_;
    std::map<Pin*,int> roots_;
    std::set<Instance*> cut_volume_;
    std::set<Pin*> pin_visited_;
    std::vector<Pin*> ordered_leaves_;
    std::vector<Pin*> ordered_roots_;
    int cut_id_;
};

}
