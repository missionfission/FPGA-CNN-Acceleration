


#include<maxpool.hpp>

void max_pool(data_t * in, layer_t layer) {
	int i,j,c,ii,jj;
	//pooling_t params = l.pool_params;

data_t * img = &in[layer.mem_addr_input];
data_t * out = &in[layer.mem_addr_output];

for (c = 0; c < layer.channels_in; c++) {
#pragma HLS LOOP_TRIPCOUNT min=64 max=256 avg=128
		for (i = 0; i < layer.height; i+=layer.stride) {
#pragma HLS LOOP_TRIPCOUNT min=14 max=224 avg=56
			for (j = 0; j < layer.width; j+=layer.stride) {
#pragma HLS LOOP_TRIPCOUNT min=14 max=224 avg=56
#pragma HLS PIPELINE II=1
				data_t max = std::numeric_limits<float>::min();
				for (ii = 0; ii < layer.kernel; ii++) {
#pragma HLS LOOP_TRIPCOUNT avg=3
					for (jj = 0; jj < layer.kernel; jj++) {
#pragma HLS LOOP_TRIPCOUNT avg=3
#pragma HLS UNROLL factor=3
						if (i+ii >= layer.height || j+jj >= layer.width) {
							continue;
						}
						data_t temp = img[(c * layer.width* layer.height) + (i+ii) * layer.width+ (j+jj)];
						if (temp > max) {
							max = temp;
						}
					}
				}
				// TODO change the parameters and verify the function
				//out[(c * div_ceil(l.h, params.stride) * div_ceil(l.w,params.stride)) + (i/params.stride * div_ceil(l.w,params.stride) + j/params.stride)] = max;
			}
		}
	}
}

void conv_pool(data_t* in, layer_t l) {
	int i; int j; int ii; int jj; int filt_num; int c;

	data_t img_buffer[224*224];
//	conv_pool_helper(in, l, img_buffer);
}
// TODO work out the changes the conv_pool_helper for the below functions
//void conv_pool_helper(data_t* in, layer_t l, data_t img_buffer[224*224]) {
//#pragma HLS INLINE
//#pragma HLS RESOURCE variable=img_buffer core=RAM_1P_BRAM
//#pragma HLS INTERFACE ap_memory port=img_buffer
//
//int i, j, ii, jj, filt_num, c;
//data_t* w = &in[l.weight_offset];
//data_t* img = &in[l.img_offset];
//data_t* out = &in[l.output_offset];
//int bias_offset = l.weight_offset + l.num_weights - l.chan_out;
//
//	for (filt_num = 0; filt_num < l.chan_out; filt_num++) {
//#pragma HLS LOOP_TRIPCOUNT min=16 max=1000 avg=256
//		data_t bias = in[bias_offset + filt_num];
//
//		data_t w_buffer[9*512*2];
//		for (i = 0; i < l.chan_in * l.k * l.k; i++) {
//#pragma HLS LOOP_TRIPCOUNT min=1 max=4608
//			w_buffer[i] = w[(filt_num * l.chan_in * l.k * l.k) + i];
//		}
//
//		for (i = 0; i < l.h; i+= l.stride){
//#pragma HLS LOOP_TRIPCOUNT min=14 max=224 avg=56
//			for (j = 0; j < l.w; j+= l.stride) {
//#pragma HLS LOOP_TRIPCOUNT min=14 max=224 avg=56
//				data_t temp = 0;
//				for (c = 0; c < l.chan_in; c++) {
//#pragma HLS LOOP_TRIPCOUNT min=3 max=512 avg=256
//#pragma HLS PIPELINE II=1
//
//					for (ii = -l.k/2; ii <= l.k/2; ii++) {
//#pragma HLS LOOP_TRIPCOUNT avg=2
//						for (jj = -l.k/2; jj <= l.k/2; jj++) {
//#pragma HLS LOOP_TRIPCOUNT avg=2
//							temp += pad_img(img, l, i+ii , j+jj, c) * w_buffer[(c * l.k * l.k) + (jj+l.k/2) * l.k + (ii+l.k/2)];
//						}
//					}
//				}
//
//				temp += bias;
//
//				if (l.relu && temp < 0.0) {
//					temp = 0.0;
//				}
//
//				img_buffer[(i/l.stride * div_ceil(l.w,l.stride) + j/l.stride)] = temp;
//			}
//		}
//
//		int conv_h = div_ceil(l.h, l.stride);
//		int conv_w = div_ceil(l.w, l.stride);
//		pooling_t params = l.pool_params;
//
//		for (i = 0; i < conv_h; i+=params.stride) {
//#pragma HLS LOOP_TRIPCOUNT min=14 max=224 avg=56
//			for (j = 0; j < conv_w; j+=params.stride) {
//#pragma HLS LOOP_TRIPCOUNT min=14 max=224 avg=56
//#pragma HLS PIPELINE II=1
//				data_t max = std::numeric_limits<float>::min();
//				for (ii = 0; ii < params.k; ii++) {
//#pragma HLS LOOP_TRIPCOUNT avg=3
//					for (jj = 0; jj < params.k; jj++) {
//#pragma HLS LOOP_TRIPCOUNT avg=3
//#pragma HLS UNROLL factor=3
//						if (i+ii >= conv_h || j+jj >= conv_w) {
//							continue;
//						}
//						data_t temp = img_buffer[(i+ii) * conv_w + (j+jj)];
//						if (temp > max) {
//							max = temp;
//						}
//					}
//				}
//				out[(filt_num * div_ceil(conv_h, params.stride) * div_ceil(conv_w,params.stride)) + (i/params.stride * div_ceil(conv_w,params.stride) + j/params.stride)] = max;
//			}
//		}
//	}
//}
