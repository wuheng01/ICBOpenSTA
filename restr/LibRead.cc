/*
Physical logic restructuring: Liberty reader
*/

#include <LibRead.hh>
#include <ExtraUtil.hh>
#include <base/main/main.h>

using namespace abc;

namespace sta {

  //Read in the liberty library using abc
  
  LibRead::LibRead(char* lib_file_name,bool verbose):
    lib_file_name_(lib_file_name), verbose_(verbose){
    Abc_Start();
    std::string cmd("read_lib ");
    std::string cmd_line = cmd + lib_file_name_;
    Abc_Frame_t* pAbc = Abc_FrameGetGlobalFrame();
    Cmd_CommandExecute(pAbc,cmd_line.c_str());
    //Check we have a library
    if (Abc_FrameReadLibGen() == NULL )
      printf("Error. Library is not available\n");
    else
      printf("Successfully read in library\n");
  }
}
