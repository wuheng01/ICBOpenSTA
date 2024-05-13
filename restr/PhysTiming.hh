/*
Physical Remapping: Timing interface
*/

#pragma once


#include <Cut.hh>

namespace sta {
  
  class Pin;

  typedef struct TimingRecord {
    float required_rise;
    float required_fall;
    float arrival_rise;
    float arrival_fall;
    float load;
  } *TimingRecordP, TimingRecordStr;

    
  class PhysTiming {
  public:
    static void AnnotateCutTiming(Cut* cut,
				  std::vector<std::pair<const Pin*, TimingRecordP>> &timing_requirements);
  };
  

}
