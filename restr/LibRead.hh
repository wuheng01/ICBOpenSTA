#pragma once

#include <vector>
#include <Cut.hh>
#include <Cut2Ntk.hh>
#include <base/abc/abc.h>
#include <map/amap/amapInt.h>
#include <base/io/ioAbc.h>

namespace abc {
  struct Amap_Lib_t_;
}

namespace sta {

  class PhysRemap;
  
  class LibRead{
  public:
    LibRead(char* lib_file_name,bool verbose);
    
  private:
    char* lib_file_name_;
    bool verbose_;
    abc::Amap_Lib_t_ *lib_;

  friend class PhysRemap;
  };
}
