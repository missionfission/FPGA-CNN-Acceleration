#include "cpu_top.hpp"


// Pointers to Shared DRAM Memory
char *SHARED_DRAM;
// layer_t *SHARED_DRAM_LAYER_CONFIG;
float *SHARED_DRAM_LAYER_CONFIG;
data_t *SHARED_DRAM_WEIGHTS;
data_t *SHARED_DRAM_DATA;


int main() {
  LOG_LEVEL = 0;

  if (!do_unittests()) {
    printf("UNIT TESTS FAILED, ABORTING.");
    return -1;
  };

  // Generate + Load Network Config from network.hpp/network.cpp
  network_t *net_CPU;
  net_CPU = get_network_config();
  setup_FPGA(net_CPU);

  // Allocate Memory on CPU Side:
  layer_t layer0 = net_CPU->layers[0];
  int win = layer0.width;
  int hin = layer0.height;
  int chin = layer0.channels_in;
  data_t *input_image = (data_t *)malloc(win * hin * chin * sizeof(data_t));

  // Load Input Image
  load_prepared_input_image(input_image, "input.bin", win, hin, chin);

  // Copy onto FPGA
  copy_input_image_to_FPGA(net_CPU, input_image);



L_LAYERS:
  for (int layer_id = 0; layer_id < net_CPU->num_layers; layer_id++) {
    layer_t layer = net_CPU->layers[layer_id];
    LOG("Layer %2d: <%s>\n", layer_id, layer.name);
    LOG_LEVEL_INCR;

    // Calculate Memory Pointers
    LOG("SHARED_DRAM is at address: %lu\n", (long)SHARED_DRAM);
    int weights_offset =
        ((long)SHARED_DRAM_WEIGHTS - (long)SHARED_DRAM) / sizeof(data_t);
    int input_offset =
        ((long)SHARED_DRAM_DATA - (long)SHARED_DRAM) / sizeof(data_t);
    numfilterelems_t weights_per_filter = (layer.kernel == 3) ? 9 : 1;
    weightaddr_t num_weights = layer.channels_in * layer.channels_out * weights_per_filter;

    // Print some Info on this Layer
    printf("CPU: Offload CONV Layer ");
    print_layer(&layer);

    printf("Entering FPGA \n");
    fflush(stdout);
    // Offload Layer Calculation to FPGA
    fpga_top(layer, (data_t *)SHARED_DRAM, weights_offset, num_weights, input_offset);

    LOG_LEVEL_DECR;
  }

  LOG_LEVEL = 0;

  // ===============================
  // = Copy Results back from FPGA =
  // ===============================
  layer_t *final = &net_CPU->layers[net_CPU->num_layers - 1];
  int ch_out = (final->is_second_split_layer ? 2 : 1) * final->channels_out;
  //int ch_out=final->channels_out;
   data_t *results = (data_t *)malloc(ch_out * sizeof(data_t));
//  copy_results_from_FPGA(net_CPU, results, ch_out);

  return 0;

}



void allocate_FPGA_memory(network_t *net_CPU) {

  int weightsize = net_CPU->num_weights * sizeof(data_t);
  int datasize = net_CPU->total_pixel_mem * sizeof(data_t);
  int total_size = 2*(weightsize + datasize);

  // Memory Allocation
  SHARED_DRAM = (char *)malloc(total_size);

  // Pointer Redirection
  SHARED_DRAM_LAYER_CONFIG = (float *)(SHARED_DRAM + 0);
  SHARED_DRAM_WEIGHTS = (data_t *)(SHARED_DRAM);
  SHARED_DRAM_DATA = (data_t *)(SHARED_DRAM + weightsize);

  // Debug: Infos about Memory Regions
  printf("CPU: FPGA DRAM Memory Allocation:\n");
  printf("Bytes allocated: %dB (config) + %dKB (weights) + %dKB (data)\n",0, weightsize / 1024, datasize / 1024);
  printf("region: %lu – %lu\n", (long)SHARED_DRAM, (long)(SHARED_DRAM + total_size));
//
//  if (DRAM_DEPTH != (int)(total_size / sizeof(data_t))) {
//    printf("\n\n!! ERROR !!\n");
//    printf("No big deal, but please set DRAM_DEPTH = %d in network.hpp\n\n",
//           (int)(total_size / sizeof(data_t)));
//    exit(-1);
//  }
}



void copy_config_to_FPGA(network_t *net_CPU) {
  int weightsize = net_CPU->num_weights * sizeof(data_t);
  // Info:
  printf("CPU: Copy Config + Weights to FPGA DRAM:\n");
 printf("     %dB (config) + %dKB (weights)\n", 0, weightsize / 1024);
  printf("     %dB (config) + %dkB (weights)\n", 0, weightsize / 1000);
  memcpy(SHARED_DRAM_WEIGHTS, net_CPU->weights, weightsize);
}

// Loads input_image with data from given file
// (prepared input file using convert_image.py)
void load_prepared_input_image(data_t *input_image, const char *filename,
                               int win, int hin, int chin) {
  // calculate size of input data
  int num_pixels = win * hin * chin;
  printf("CPU: Loading Input from File %s, %lu kBytes.\n", filename,
         num_pixels * sizeof(data_t) / 1024);

  // load binary data from file
  FILE *infile = fopen(filename, "rb");
  if (!infile) {
    printf("ERROR: File %s could not be opened!\n", filename);
    exit(-1);
  }
  fread(input_image, sizeof(data_t), num_pixels, infile);
  fclose(infile);
}


void copy_input_image_to_FPGA(network_t *net_CPU, data_t *image) {
  // Input Data goes into Layer 0:
  int win = net_CPU->layers[0].width;
  int hin = net_CPU->layers[0].height;
  int chin = net_CPU->layers[0].channels_in;
  int input_img_size = win * hin * chin * sizeof(data_t);

  // Info:
  printf("CPU: Copy Input Data: %dKB (input image)\n", input_img_size / 1024);

  // Copy Input Data:
  memcpy(SHARED_DRAM_DATA, image, input_img_size);
}


void copy_results_from_FPGA(network_t *net_CPU, data_t *results, int ch_out) {
  // Verify that last layer reduces spatial dimensions to 1x1:
  assert(net_CPU->layers[net_CPU->num_layers - 1].global_pool == true);

  // Assumption: Output Data is at beginning of DATA section (shared DRAM)
  int result_offset = 0;
  int result_size = ch_out * sizeof(data_t);

  printf("CPU: Copy Results from FPGA DRAM: %d Bytes\n", result_size);

  // Copy Result Data:
  memcpy(results, SHARED_DRAM_DATA + result_offset, result_size);
}

void calculate_softmax(network_t *net_CPU, data_t *results,std::vector<std::pair<data_t, int> > &probabilities) {
  layer_t *final = &net_CPU->layers[net_CPU->num_layers - 1];
  data_t num_output_pixels = final->width * final->height;
  if (final->stride == 2) num_output_pixels /= 4;
  int ch_out = final->channels_out;
  if (final->is_second_split_layer) ch_out *= 2;

  data_t maxresult = 0;
  for (int i = 0; i < ch_out; i++) {
    // Average over spatial output dimensions
    results[i] /= num_output_pixels;
    // Find maximum result
    maxresult = std::max(maxresult, results[i]);
    DBG("  %6.2f\n", results[i]);
  }
  DBG("(maximum: %6.2f)\n", maxresult);

  // Calculate Exponentials and Sum
  data_t expsum = 0;
  std::vector<data_t> exponentials(ch_out);
  for (int i = 0; i < ch_out; i++) {
    // Subtract Maximum ("normalize"), then calculate e^()
    exponentials[i] = std::exp(results[i] - maxresult);
    // Accumulate Sum of Exponentials
    expsum += exponentials[i];
  }
  DBG("sum of exponentials: %f\n", expsum);

  // Calculate Softmax Probabilities [ p_i = e^{r_i} / (\sum(e^{r_i})) ]
  for (int i = 0; i < ch_out; i++) {
    probabilities[i] = std::pair<data_t, int>(exponentials[i] / expsum, i);
    // printf("P(class %3d) = %4.2f%%\n", i, 100 * probabilities[i].first);
  }

  // Sort (small index = high probability)
  std::sort(probabilities.begin(), probabilities.end());
  std::reverse(probabilities.begin(), probabilities.end());
}


void load_image_file(data_t *input_image, const char *filename, int win,int hin, int chin) {
  
  
  
  }


// void do_preprocess(data_t *input_image, int win, int hin, int chin) {
//   for (int y = 0; y < hin; y++) {
//     for (int x = 0; x < win; x++) {
//       // Subtract Mean Pixel (defined in network.hpp)
//       input_image[y * win * chin + x * chin + 0] -= MEAN_R;
//       input_image[y * win * chin + x * chin + 1] -= MEAN_G;
//       input_image[y * win * chin + x * chin + 2] -= MEAN_B;
//     }
//   }
// }

void setup_FPGA(network_t *net_CPU) {

  printf("\n\nCPU: Network Setup:\n=====================\n\n");
  print_layers(net_CPU);
  printf("\n");

  allocate_FPGA_memory(net_CPU);
  copy_config_to_FPGA(net_CPU);

}
