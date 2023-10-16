%module verilog

%{

// OpenSTA, Static Timing Analyzer
// Copyright (c) 2022, Parallax Software, Inc.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include "VerilogReader.hh"
#include "VerilogWriter.hh"
#include "Sta.hh"
#include <CutGen.hh>
#include <PhysTiming.hh>  
#include <PhysRemap.hh>
#include <ClusterCutGen.hh>

using sta::Sta;
using sta::NetworkReader;
using sta::readVerilogFile;

%}

%inline %{

bool
read_verilog_cmd(const char *filename)
{
  Sta *sta = Sta::sta();
  NetworkReader *network = sta->networkReader();
  if (network) {
    sta->readNetlistBefore();
    return readVerilogFile(filename, network);
  }
  else
    return false;
}

void
delete_verilog_reader()
{
  deleteVerilogReader();
}

void
write_verilog_cmd(const char *filename,
		  bool sort,
		  bool include_pwr_gnd,
		  bool exclude_not_in_lib,
		  CellSeq *remove_cells)
{
  // This does NOT want the SDC (cmd) network because it wants
  // to see the sta internal names.
  Sta *sta = Sta::sta();
  Network *network = sta->network();
  writeVerilog(
    filename, sort, include_pwr_gnd, 
    exclude_not_in_lib, remove_cells, network
  );
  delete remove_cells;
}


   //
  //Physical Synthesis code.
  //

  //
  //Cut Generation
  //Shows how to build a cut and get its logic signature
  //Application: small cuts which can map to single gates
  //using oracle. 
  //
 
  void
    write_cuts_cmd( char* file_name){
    FILE* fp = fopen(file_name,"w+");
    LibertyLibrary* cut_library = new LibertyLibrary("cut_library","cut1.txt");
    
    if (fp){
      Sta *sta = Sta::sta();
      Network *network = sta->network();
      std::vector<Cut* > cut_set;
      LeafInstanceIterator* inst_iterator = network -> leafInstanceIterator();
      while (inst_iterator -> hasNext()){
	Instance* cur_inst = inst_iterator -> next();
	CutGen cut_gen(network,cut_library);
	cut_gen.GenerateInstanceCutSet(cur_inst,cut_set);
      }
      for (auto c: cut_set)
	c -> Print(fp,network);
      fclose(fp);
    }
  }

  //
  //Cut Physical Synthesis.
  //---------------------
  //
  //Application: Large multiple output cuts ("blobs") of logic
  //with timing constraints.
  //
  //Source level integration (with timing) version of restructure
  //command in OpenRoad (no blif interface needed, real timing
  //constraints passed in, fine control over remapping).
  //
  
  void
    phys_remap_cmd(char* target_library,char* script_file_name, bool verbose){
  
    //Read in the Library library

    //TODO: Make this a separate command to avoid repeated re-reading
    printf("Reading in abc library %s\n",target_library);
    LibRead liberty_library(target_library,verbose);

    //
    //generate the cutset.
    //pick a cut and remap it.
    //
    //
    //TODO: Generate cut from timing. The following code
    //is just to show how to get a cut using what we have now.
    //It is ok just to carve out a whole chunk of the design
    //and stuff it in a cut.
    //
  
    Cut* cut_to_remap=nullptr;

    Sta *sta = Sta::sta();
    Network *network = sta->network();

#ifdef TEST_CLUSTER_CUT
    //generate the cluster cuts for resynthesis.
    ClusterCutGen cg(network);
    cg.GenClusters();
    //cut set is in
    //cg.cutset()
#endif
    
    {
      //All of the stuff below is just to generate a cut...
      //ok to kill all this code and replace with cut extraction
      //from sta world.
      LibertyLibrary* cut_library = new LibertyLibrary("cut_library","cut1.txt");
      std::vector<Cut* > cut_set;
      LeafInstanceIterator* inst_iterator = network -> leafInstanceIterator();
      while (inst_iterator -> hasNext()){
	Instance* cur_inst = inst_iterator -> next();
	CutGen cut_gen(network,cut_library);
	cut_gen.GenerateInstanceCutSet(cur_inst,cut_set);
      }
      int max_size=0;
      for (auto c: cut_set){
	if (c -> volume_.size() > max_size)
	  cut_to_remap = c;
      }
    }
  
    //
    //We are going to remap "cut_to_remap"
    //
  
    if (cut_to_remap){
      //Get the timing numbers for the cut from the sta
      //Explicitly pass them into optimizer
      std::vector<std::pair<const Pin*, TimingRecordP> > timing_requirements;
      PhysTiming::AnnotateCutTiming(cut_to_remap, timing_requirements);
      //Remap the cut with the physical timing constraints
      //onto the target library.
      bool script_present=false;
      std::string script_name;
      printf("Script file_name %s of length %d\n",script_file_name,
	     strlen(script_file_name));
      if (script_file_name && strlen(script_file_name)>0){
	script_name = script_file_name;
	script_present=true;
      }
      PhysRemap prm(network,
		    cut_to_remap,
		    timing_requirements,
		    target_library,
		    liberty_library,
		    script_present,
		    script_name
		    );
      prm.Remap();
    }
  }


%} // inline
