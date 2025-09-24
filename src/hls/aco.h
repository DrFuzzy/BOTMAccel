#ifndef ACO_H
#define ACO_H

#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include <iostream>

#ifdef DEBUG
#define DEBUG_PRINT(x) std::cout << "[DEBUG] " << x << std::endl
#else
#define DEBUG_PRINT(x)
#endif

// ACO Parameters
#ifndef DIMENSIONS
#warning "No DIMENSIONS set (e.g. -DDIMENSIONS=6), defaulting to DIMENSIONS=4"
#define DIMENSIONS 4 // Number of dimensions
#endif

#if DIMENSIONS != 4 && DIMENSIONS != 6 && DIMENSIONS != 8
#error                                                                         \
    "Invalid DIMENSIONS value. DIMENSIONS must be defined as 4, 6, or 8 (e.g. -DDIMENSIONS=6)"
#endif

#ifndef PARAM_SET // Parameter range selection
#warning "No PARAM_SET set (e.g. -DPARAM_SET=0), defaulting to DPARAM_SET=0"
#define PARAM_SET 0   // 0 = default, 1 = real-world CSV params (SAMPLING_TIME=9.846666667)
#endif

#define NUM_ANTS 20         // Number of ants
#define ITERATIONS 1000     // Maximum number of iterations
#define EVAPORATION_RATE 0.1 // Pheromone evaporation rate

#ifndef RANDOM_SEED
#warning "No RANDOM_SEED set (e.g. -DRANDOM_SEED=12345), defaulting to RANDOM_SEED=1"
#define RANDOM_SEED 1 // RNG seed
#endif

#define MAX_ENTRIES 1801     // Adjust this as per your actual data size

#ifndef SAMPLING_TIME // Sampling time
#warning "No SAMPLING_TIME set (e.g. -DSAMPLING_TIME=2), defaulting to SAMPLING_TIME=2"
#define SAMPLING_TIME 2 // RNG seed
#endif

// AXI Stream data type (32-bit for output, 96-bit for input)
typedef ap_axiu<96, 1, 1, 1> axis_in_t;
typedef ap_axiu<32, 1, 1, 1> axis_out_t;

extern "C" {

void aco(hls::stream<axis_in_t> &in_stream, 
         hls::stream<axis_out_t> &out_stream, int n);
}

#endif // ACO_H
