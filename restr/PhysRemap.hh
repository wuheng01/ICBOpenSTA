/*
Remap a cut
*/

#pragma once

#include <vector>
#include <Cut.hh>
#include <Cut2Ntk.hh>
#include <base/abc/abc.h>
#include <LibRead.hh>

namespace abc {
typedef struct Abc_Ntk_t_ Abc_Ntk_t;
typedef struct Abc_Obj_t_ Abc_Obj_t;
typedef struct Amap_Lib_t_ Amap_Lib_t;
}

namespace sta {

  class Network;
  class Pin;
  class Instance;


  
  class PhysRemap{
  public:
    PhysRemap(Network* nwk,
	      Cut* cut_to_remap,
	      std::vector<std::pair<const Pin*, TimingRecordP> > & timing_requirements,
	      char* lib_name,
	      LibRead& lib_file,
	      bool script,
	      std::string &script_to_apply
	      ):nwk_(nwk),
		cut_(cut_to_remap),
		timing_requirements_(timing_requirements),
		lib_name_(lib_name),
		liberty_library_(lib_file.lib_),
		script_(script),
		script_to_apply_(script_to_apply)
    {
      std::string cut_name_ = "cut_" + std::to_string(cut_to_remap -> id_);
      global_optimize_ = false;
      verbose_=true;
    }
    void Remap();
    void AnnotateTimingRequirementsOntoABCNwk(abc::Abc_Ntk_t* sop_logic_nwk);
    void BuildCutRealization(abc::Abc_Ntk_t*);
    
  private:
    Network* nwk_;
    Cut* cut_;
    std::vector<std::pair<const Pin*, TimingRecordP> > & timing_requirements_;
    char* lib_name_;
    std::string cut_name_;

    abc::Amap_Lib_t* liberty_library_;

    bool global_optimize_;
    bool verbose_;
    bool script_;
    std::string script_to_apply_;
  };
}
