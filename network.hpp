// Network + Layer Configuration.

#ifndef _NETWORK_H_
#define _NETWORK_H_
//
//// Size Limits for this Network
//const int MAX_NUM_LAYERS = 27;
//const int MAX_WEIGHTS_PER_LAYER = 387184;
//const int MAX_IMAGE_CACHE_SIZE = 1024;
//const int MAX_INPUT_PER_LAYER = 1048576;
//const int MAX_NUM_CHOUT = 512;
//const int MAX_DIMENSION = 256;
//const int MAX_CHANNELS = 736;
//
//const int TOTAL_NUM_WEIGHTS = 2528800;
//const int TOTAL_NUM_INPUTS = 3661824;
//const int TOTAL_NUM_OUTPUTS = 3239936;
//const int TOTAL_DRAM_IO = 9430560;
//
//const int DRAM_DEPTH = 11865152;
//const float TEST_RESULT_EXPECTED = 88.38;//
const int MAX_NUM_LAYERS = 15;
const int MAX_WEIGHTS_PER_LAYER = 36992*2;
const int MAX_IMAGE_CACHE_SIZE = 1024*2;
const int MAX_INPUT_PER_LAYER = 16384*2;
const int MAX_NUM_CHOUT = 256;
const int MAX_DIMENSION = 16;
const int MAX_CHANNELS = 256;

const int TOTAL_NUM_WEIGHTS = 153186;



const int DRAM_DEPTH = 11000;
//const float TEST_RESULT_EXPECTED = 88.38;

// Mean Pixel for ImageNet Data
const float MEAN_R = 104;
const float MEAN_G = 117;
const float MEAN_B = 123;

// include after definitions to allow bit-width calculations
#include "netconfig.hpp"

network_t *get_network_config();

#endif
