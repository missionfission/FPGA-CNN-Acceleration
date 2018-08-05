//------------------------------------------------------------------------------
//  SqueezeNetOnFPGA
//------------------------------------------------------------------------------


#ifndef GPool_CACHE_HPP_07571FC2
#define GPool_CACHE_HPP_07571FC2

// Data Types for FPGA Implementation
#include "fpga_top.hpp"

namespace GPoolCache {
  void accumulateChannel(channel_t co, data_t data);
  data_t getChannel(channel_t c);
  void setChannel(channel_t c, data_t data);
  void reset();

  extern data_t GBRAM[MAX_NUM_CHOUT];
};

#endif
