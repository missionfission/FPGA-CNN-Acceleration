//------------------------------------------------------------------------------
//  SqueezeNetOnFPGA
//------------------------------------------------------------------------------
//
//	File:  netconfig.cpp
//
//  addLayer(), loadWeightsFromFile(), print_layer(), print_layers()
//
//-------------------------------------------------------------------------------

#include "network.hpp"
#include "netconfig.hpp"
#include<fstream>
#include<iostream>
// ==============================
// = Add Layer to given Network =
// ==============================

void addLayer(network_t *net, layer_t layer) {
  // Assumes that Network has been initialized with enough memory (not checked!)
  // Uses static variables -> can only be used for 1 network definition per
  // program!
  // If layer.is_first_split_layer==true, reserves double amount of
  //     output memory to allow implicit output-channel concatenation.
  //     Use for (expand1x1) and other "output-channel split" layers.
  // If layer.is_second_split_layer==true, uses same input memory address as in
  //     last layer, and moves output address only by layer.channels_out/2
  //     to interleave with previous layer's output.
  //     Use for (expand3x3) and other "output-channel split" layers.

  // Keep Track of Memory Locations for Activations + Weights (-> static)
  static int current_output_addr = 0;
  static int current_input_addr = 0;
  static int current_weights_addr = 0;
  if(layer.width!=0)
  // Align to memory borders (float needed because of ceil() operation below)
  { float mem_border = MEMORY_ALIGNMENT / sizeof(data_t);

  // Data Size Calculations
  int input_data_pixels = layer.width * layer.height * layer.channels_in;
  int width_out =
      1 + std::floor((float)(layer.width + 2 * layer.pad - layer.kernel) /
                     layer.stride);
  int height_out =
      1 + std::floor((float)(layer.height + 2 * layer.pad - layer.kernel) /
                     layer.stride);
  int output_data_pixels = width_out * height_out * layer.channels_out;
  int num_weights =  // conv + bias weights
      layer.channels_out * layer.channels_in * layer.kernel * layer.kernel +
      layer.channels_out;

  if (layer.is_second_split_layer == false) {
    // For "normal" layers, and "1st split" layers:
    // Update Input + Output Memory Address
    // - read input from previous layer's output address
    // - write output to next free memory position

    // this layer's input memory = last layer's output memory
    current_input_addr = current_output_addr;
    layer.mem_addr_input = current_input_addr;

    // this layer's output comes right next, aligned to memory border
    current_output_addr += input_data_pixels;
    current_output_addr = std::ceil(current_output_addr / mem_border) * mem_border;
    layer.mem_addr_output = current_output_addr;

  } else {
    // For "2nd split" layer (expand3x3, conv10/split2):
    // - read from same input location
    // - interleave output channels (write to slightly offset memory address)

    // current_output_addr + current_input_addr is still as in last layer...
    layer.mem_addr_output = current_output_addr + layer.channels_out;
    layer.mem_addr_input = current_input_addr;
  }

  /*printf("layer %s, update_memory_address = %d, current_input_addr = %d, "
   "current_output_addr = %d", layer.name, update_memory_address,
   current_input_addr, current_output_addr);*/

  // Store + Update Weights Memory Address
  // (don't align, weights are read sequentially)
  layer.mem_addr_weights = current_weights_addr;
  current_weights_addr += num_weights;

  net->layers[net->num_layers] = layer;
   net->num_layers++;
   net->total_pixel_mem = current_output_addr + output_data_pixels;
  }
  else
  {
	  float mem_border = MEMORY_ALIGNMENT / sizeof(data_t);
	  int input_data_pixels=layer.channels_in;
	  int output_data_pixels=layer.channels_out;
	  int num_weights =  layer.channels_in*layer.channels_out+ layer.channels_out;
  	 current_input_addr = current_output_addr;
     layer.mem_addr_input = current_input_addr;

     // this layer's output comes right next, aligned to memory border
     current_output_addr += input_data_pixels;
     current_output_addr = std::ceil(current_output_addr / mem_border) * mem_border;
     layer.mem_addr_output = current_output_addr;

     net->layers[net->num_layers] = layer;
       net->num_layers++;
       net->total_pixel_mem = current_output_addr + output_data_pixels;

  }
  // Write Options into Layer Config
  /* layer.is_expand_layer = is_expand_layer;
   layer.pool = pool_type;*/

  // Add Layer to network

};

// =================================
// = Load Weights from Binary File =
// =================================
// prepare with convert_caffemodel.py

void loadWeightsFromFile(network_t *net, const char *filename) {
  FILE *filehandle = fopen(filename, "rb");
  if (!filehandle) {
    printf("ERROR: File %s could not be opened!\n", filename);
    exit(-1);
  }

  for (int i = 0; i < net->num_layers; i++) {
    layer_t *layer = &net->layers[i];


    int chout = layer->channels_out;
    int chin = layer->channels_in;
    int kernel = layer->kernel;
    if(layer->width!=0) {
    // calculate address within weight memory section
    int num_weights = chout * chin * kernel * kernel + chout;
   data_t *weights_addr = (net->weights + layer->mem_addr_weights);
    //printf(i);
  //  fread(weights_addr, sizeof(data_t), num_weights, filehandle);
    }
    else
    {
    	 int num_weights = chout * chin  + chout;
    	 data_t *weights_addr = (net->weights + layer->mem_addr_weights);
    	 fread(weights_addr, sizeof(data_t), num_weights, filehandle);


    }
    // read portion of input file
   // fread(weights_addr, sizeof(data_t), num_weights, filehandle);
  }

  fclose(filehandle);
}

// =========================================
// = Print Overview Table of given Network =
// =========================================
// Print List of all Layers + Attributes + Memory Locations
#define use_KB 0
#if use_KB
#define unit "k"
#define divi 1024
#else
#define unit ""
#define divi 1
#endif

void print_layer(layer_t *layer) {
  int memory_needed = layer->height * layer->width * layer->channels_in;
  int weights_size =
      layer->kernel * layer->kernel * layer->channels_in * layer->channels_out +
      layer->channels_out;

  printf("%6s: %3dx%-3d x %3d > %-3d, ", layer->name, (int)layer->height,
         (int)layer->width, (int)layer->channels_in, (int)layer->channels_out);

  printf("CONV (%dx%d)/%d%s %s, ", (int)layer->kernel, (int)layer->kernel,
         (int)layer->stride, layer->pad ? "p" : " ",
         layer->relu ? "+ ReLU" : "      ");

  printf("IN @mem(%8lu-%8lu" unit "B), ",
         long(layer->mem_addr_input * sizeof(float) / divi),
         long((layer->mem_addr_input + memory_needed) * sizeof(float) / divi));

  printf("OUT @mem(%8lu" unit "B), ",
         long(layer->mem_addr_output * sizeof(float) / divi));

  printf("WEIGHTS @mem(%8lu-%8lu" unit "B)",
         long(layer->mem_addr_weights * sizeof(float) / divi),
         long((layer->mem_addr_weights + weights_size) * sizeof(float) / divi));

  if (layer->is_second_split_layer | layer->is_first_split_layer)
    printf(" (split%d)", (layer->is_first_split_layer) ? 1 : 2);

  if (layer->global_pool == true) printf(" GLOBAL POOL");

  printf("\n");
};

void print_layers(network_t *net) {
  for (int i = 0; i < net->num_layers; i++) {
    layer_t *layer = &net->layers[i];
    print_layer(layer);
  }
}
