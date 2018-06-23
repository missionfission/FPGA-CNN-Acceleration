
///////
// Network + Layer Configuration.
// File automatically generated by convert_caffemodel.py
// from deploy.prototxt and snapshot_iter_240240.caffemodel
// (c) 2016 David Gschwend
///////

#include "network.hpp"
network_t *get_network_config() {
  network_t *net = new network_t(13, 40000);

  // Layer Attributes: ( NAME   ,   W,   H,   CI,  CO, K, P, S, R, S1, S2, FP)
  addLayer(net, layer_t("c1    ",  16,   16,    3,  64, 3, 1, 2, 1,  0,  0,  0));
  addLayer(net, layer_t("f2/s1 ",   8,   8,    64,  16, 1, 0, 1, 1,  0,  0,  0));
  addLayer(net, layer_t("f2/e1 ",   8,   8,   16,  64, 1, 0, 1, 1,  1,  0,  0));
  addLayer(net, layer_t("f2/e3 ",   8,   8,   16,  64, 3, 1, 1, 1,  0,  1,  0));
  addLayer(net, layer_t("f3/s1 ",  8,   8,  128,  16, 1, 0, 1, 1,  0,  0,  0));
  addLayer(net, layer_t("f3/e1 ",  8,   8,   16,  64, 1, 0, 1, 1,  1,  0,  0));
  addLayer(net, layer_t("f3/e3 ",   8,   8,   16,  64, 3, 1, 1, 1,  0,  1,  0));
  addLayer(net, layer_t("f4/s1 ",   8,   8,  128,  32, 1, 0, 1, 1,  0,  0,  0));
  addLayer(net, layer_t("f4/e1 ",   8,   8,   32, 128, 1, 0, 1, 1,  1,  0,  0));
  addLayer(net, layer_t("f4/e3 ",   8,   8,   32, 128, 3, 1, 1, 1,  0,  1,  0));
  addLayer(net, layer_t("f5/s1 ",   8,   8,  256,  32, 1, 0, 1, 1,  0,  0,  0));
  addLayer(net, layer_t("f5/e1 ",   8,   8,   32, 128, 1, 0, 1, 1,  1,  0,  0));
  addLayer(net, layer_t("f5/e3 ",   8,   8,   32, 128, 3, 1, 1, 1,  0,  1,  0));
  addLayer(net, layer_t("dense ",   0,   0,   16384, 2, 0, 0, 0, 0,  0,  0,  0));

  net->num_weights = 120416;
  const char* filename = "weights_malaria.bin";
  loadWeightsFromFile(net, filename);
  return net;
}
