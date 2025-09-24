// timescale 1ns/1ps
#include "aco.h"
#include <cmath>
//#include <hls_math.h>
#include <cstdint>
#include <limits>

#define ATAN_LUT_SIZE 1024
static float atan_lut[ATAN_LUT_SIZE];

// Parameter ranges
#if DIMENSIONS == 4
float pheromones[DIMENSIONS] = {1.0f, 1.0f, 1.0f, 1.0f};
#if PARAM_SET == 0
const float parameter_ranges[4][2] = {
    {20000, 40000}, // theta[0]
    {20000, 40000}, // theta[1]
    {5, 10},        // theta[2]
    {5, 10},        // theta[3]
};
#elif PARAM_SET == 1
const float parameter_ranges[4][2] = {
    {-500000, -200000}, // theta[0]
    {2000000, 5000000}, // theta[1]
    {0, 10},        // theta[2]
    {0, 10},        // theta[3]
};
#else
#error "Invalid PARAM_SET value. Must be 0, or 1."
#endif

#elif DIMENSIONS == 6
float pheromones[DIMENSIONS] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
#if PARAM_SET == 0
const float parameter_ranges[6][2] = {
    {20000, 40000}, // theta[0]
    {20000, 40000}, // theta[1]
    {5, 10},        // theta[2]
    {5, 10},        // theta[3]
    {-0.01, 0.01},  // theta[4]
    {-0.01, 0.01},  // theta[5]
};
#elif PARAM_SET == 1
const float parameter_ranges[6][2] = {
    {-500000, -200000}, // theta[0]
    {2000000, 5000000}, // theta[1]
    {0, 10},        // theta[2]
    {0, 10},        // theta[3]
    {0, 0.001},  // theta[4]
    {-0.001, 0},  // theta[5]
};
#else
#error "Invalid PARAM_SET value. Must be 0, or 1."
#endif

#elif DIMENSIONS == 8
float pheromones[DIMENSIONS] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
#if PARAM_SET == 0
const float parameter_ranges[8][2] = {
    {20000, 40000},    // theta[0]
    {20000, 40000},    // theta[1]
    {5, 10},           // theta[2]
    {5, 10},           // theta[3]
    {-0.01, 0.01},     // theta[4]
    {-0.01, 0.01},     // theta[5]
    {-0.0001, 0.0001}, // theta[6]
    {-0.0001, 0.0001}, // theta[7]
};
#elif PARAM_SET == 1
const float parameter_ranges[8][2] = {
    {-500000, -200000}, // theta[0]
    {2000000, 5000000}, // theta[1]
    {0, 10},        // theta[2]
    {0, 10},        // theta[3]
    {0, 0.001},  // theta[4]
    {-0.001, 0},  // theta[5]
    {-0.00001, 0.00001},  // theta[6]
    {-0.00001, 0.00001},  // theta[7]
};
#else
#error "Invalid PARAM_SET value. Must be 0, or 1."
#endif

#else
#error "Invalid DIMENSIONS value. Must be 4, 6, or 8."
#endif

void init_atan_lut() {
    for (int i = 0; i < ATAN_LUT_SIZE; i++) {
        float ratio = (float)i / (ATAN_LUT_SIZE - 1); // 0..1
        atan_lut[i] = atan(ratio); // radians
    }
}

float fast_atan2(float y, float x) {
    #pragma HLS INLINE
    float abs_y = (y >= 0) ? y : -y;
    float abs_x = (x >= 0) ? x : -x;

    float ratio;
    if (abs_x > abs_y) {
        ratio = abs_y / abs_x;
    } else {
        ratio = abs_x / abs_y;
    }

    // LUT index
    int index = (int)(ratio * (ATAN_LUT_SIZE - 1));
    if (index >= ATAN_LUT_SIZE) index = ATAN_LUT_SIZE - 1;

    float angle = atan_lut[index];

    // Quadrant correction
    if (abs_x > abs_y) {
        angle = (x >= 0) ? ((y >= 0) ? angle : -angle) : ((y >= 0) ? M_PI - angle : -M_PI + angle);
    } else {
        angle = (x >= 0) ? ((y >= 0) ? M_PI/2 - angle : -M_PI/2 + angle) : ((y >= 0) ? M_PI/2 + angle : -M_PI/2 - angle);
    }

    return angle;
}

// // Random number generator (for simplicity, linear congruential generator)
// float random_float() {
// #pragma HLS INLINE
//   random_state = random_state * 1664525 + 1013904223;
//   return (random_state % 1000) / 1000.0f; // Generate float in [0, 1)
// }

// float random_float(unsigned int &random_state) {
//     #pragma HLS INLINE
//     random_state = random_state * 1664525 + 1013904223;
//     return (random_state % 1000) / 1000.0f; // Generate float in [0, 1)
// }

// 32-bit PCG-like generator (HLS-friendly)
// static uint64_t rng_state = 0x853c49e6748fea9bULL;
// //static uint64_t rng_state = RANDOM_SEED;
// static const uint64_t rng_mult = 6364136223846793005ULL;
// static const uint64_t rng_inc  = 1442695040888963407ULL;

// // Output is truncated to 24-bit mantissa-like float precision.
// // You can change rng_state to accept a seed for reproducibility.
// float random_float() {
// #pragma HLS INLINE
//     rng_state = rng_state * rng_mult + rng_inc;
//     uint32_t xorshifted = ((rng_state >> 18u) ^ rng_state) >> 27u;
//     uint32_t rot = rng_state >> 59u;
//     uint32_t result = (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
//     return (result & 0xFFFFFF) / 16777216.0f; // Convert to [0,1)
// }

// LFG (Lagged Fibonacci Generator) - HLS-friendly
// static uint64_t lfg_state[2] = {RANDOM_SEED, RANDOM_SEED + 1}; // Two 64-bit
// states static const uint64_t lfg_m = 24; // Lag 1 static const uint64_t lfg_n
// = 55; // Lag 2

// float random_float() {
// #pragma HLS INLINE
//     uint64_t next = (lfg_state[0] + lfg_state[1]) % (1ULL << 64); //
//     Fibonacci sum lfg_state[0] = lfg_state[1]; // Shift states lfg_state[1] =
//     next; // Update new state

//     return (next * 0.5 / (1ULL << 64)); // Normalize to [0, 1)
// }

// Single-state Xorshift RNG (HLS-friendly)
// static uint64_t xorshift_state = RANDOM_SEED; // Single 64-bit state
// static const uint64_t xorshift_mult =
//     6364136223846793005ULL; // Multiplier for RNG
// static const uint64_t xorshift_add =    DEBUG_PRINT("Iteration " << iter + 1 << "/" << ITERATIONS
                            //  << ", Best Fitness: " << best_fitness);
  
//     1442695040888963407ULL; // Golden ratio for better randomness

// float random_float() {
// //#pragma HLS inline
//   xorshift_state =
//       xorshift_state * xorshift_mult + xorshift_add;          // Update state
//   uint32_t result = (xorshift_state >> 18u) ^ xorshift_state; // XOR shift
//   return (result & 0xFFFFFF) / 16777216.0f; // Normalize to [0,1)
// }

float random_float(uint64_t &random_state) {
  static const uint64_t xorshift_mult = 6364136223846793005ULL;
  static const uint64_t xorshift_add  = 1442695040888963407ULL;

  random_state = random_state * xorshift_mult + xorshift_add;
  uint32_t result = (random_state >> 18u) ^ random_state;
  return (result & 0xFFFFFF) / 16777216.0f;
}

void objective_function(const float theta[DIMENSIONS], const float ownship_x[],
                        const float ownship_y[], const float measure[],
                        int n, float &fitness) {
  float partial_sums[MAX_ENTRIES];
  //unsigned int timeframe = 0; 
  float timeframe = 0;
loop_obj_f:
  for (int i = 0; i < n; i++) {
    timeframe += SAMPLING_TIME; //std::cout << "timeframe: " << timeframe << '\n';

    float x_t = 0.0f;
    float y_t = 0.0f;
    //unsigned int pow = 1;
    float pow = 1.0f;
    unsigned int fact = 1;
    int order = 1;

    for (int j = 0; j < DIMENSIONS; j += 2) {
      //float gamma = static_cast<float>(pow) / static_cast<float>(fact);
      float gamma = pow / static_cast<float>(fact);
      x_t += theta[j] * gamma;
      y_t += theta[j + 1] * gamma;
      pow *= timeframe;
      fact *= order;
      order++;
    }

    //float h = hls::atan2f(y_t - ownship_y[i], x_t - ownship_x[i]);
    float h = fast_atan2(y_t - ownship_y[i], x_t - ownship_x[i]);
    float diff = measure[i] - h;
    partial_sums[i] = diff * diff;
  }

  const int REDUCTION_FACTOR = 8;
  float local_sums[REDUCTION_FACTOR] = {0};
#pragma HLS ARRAY_PARTITION variable = local_sums complete

loop_partial_accum:
  for (int i = 0; i < n; i++) {
#pragma HLS PIPELINE II = 1
    local_sums[i % REDUCTION_FACTOR] += partial_sums[i];
  }

  // Final accumulation
  float sum_squared_diff = 0.0f;
  for (int i = 0; i < REDUCTION_FACTOR; i++) {
#pragma HLS UNROLL
    sum_squared_diff += local_sums[i];
  }
  
  fitness = sum_squared_diff;
}

void read_input_stream(hls::stream<axis_in_t> &in_stream,
                       float ownship_x_arr[MAX_ENTRIES],
                       float ownship_y_arr[MAX_ENTRIES],
                       float measure_arr[MAX_ENTRIES],
                       int n) {
#pragma HLS INLINE off
  for (int i = 0; i < n; i++) {
#pragma HLS PIPELINE II = 1
    if (!in_stream.empty()) {
      axis_in_t input = in_stream.read();
      ap_uint<96> data = input.data;

      // Extract raw bits
      uint32_t raw_x = data.range(31, 0);
      uint32_t raw_y = data.range(63, 32);
      uint32_t raw_m = data.range(95, 64);

      // Reinterpret bits as floats (synthesizable in HLS)
      float ownship_x = *reinterpret_cast<float*>(&raw_x);
      float ownship_y = *reinterpret_cast<float*>(&raw_y);
      float measure   = *reinterpret_cast<float*>(&raw_m);

      // Store results
      measure_arr[i]   = measure;
      ownship_y_arr[i] = ownship_y;
      ownship_x_arr[i] = ownship_x;

      DEBUG_PRINT(i << ", "
                    << "ownship_x = " << ownship_x << ", "
                    << "ownship_y = " << ownship_y << ", "
                    << "measure = " << measure);
    }
  }
}

void compute_aco_kernel(const float ownship_x_arr[MAX_ENTRIES],
                        const float ownship_y_arr[MAX_ENTRIES],
                        const float measure_arr[MAX_ENTRIES],
                        int n,
                        hls::stream<float> &best_fitness_out,
                        hls::stream<float> &best_solution_out) {
#pragma HLS INLINE off
  // Initialize atan LUT once
  init_atan_lut();

  float best_fitness = 3.4028235e+38f;
  float current_best_solution[DIMENSIONS];
  uint64_t random_seed = static_cast<uint64_t>(RANDOM_SEED);
  //unsigned int random_state = RANDOM_SEED;

loop_iter:
  for (int iter = 0; iter < ITERATIONS; iter++) {
#pragma HLS PIPELINE off

    float ants[NUM_ANTS][DIMENSIONS];
    float fitness[NUM_ANTS];

#pragma HLS ARRAY_PARTITION variable=ants complete dim=1
#pragma HLS ARRAY_PARTITION variable=fitness complete

    // Generate and evaluate
    for (int ant = 0; ant < NUM_ANTS; ant++) {
#pragma HLS UNROLL off=false
      for (int d = 0; d < DIMENSIONS; d++) {
#pragma HLS UNROLL
        float rand = random_float(random_seed);
        // ants[ant][d] = parameter_ranges[d][0] +
        //                rand * (parameter_ranges[d][1] - parameter_ranges[d][0]);
        // bias the random value toward pheromones[d]
        float bias = pheromones[d];
        float lower = parameter_ranges[d][0];
        float upper = parameter_ranges[d][1];

        // scale rand by pheromone
        float biased_rand = powf(rand, 1.0f / (1.0f + bias));

        if (random_float(random_seed) <= 0.2) // epsilon = 0.2
          biased_rand = rand;

        ants[ant][d] = lower + biased_rand * (upper - lower);
      }

      objective_function(ants[ant], ownship_x_arr, ownship_y_arr,
                         measure_arr, n, fitness[ant]);

      if (fitness[ant] < best_fitness) {
        best_fitness = fitness[ant];
        for (int i = 0; i < DIMENSIONS; i++) {
#pragma HLS UNROLL
          current_best_solution[i] = ants[ant][i];
        }
      }
    }

    // Update pheromones
    #pragma HLS ARRAY_PARTITION variable = pheromones complete

  loop_pherom_update:
    for (int d = 0; d < DIMENSIONS; d++) {
#pragma HLS UNROLL // Optional: unroll outer loop if DIMENSIONS is small

      float pheromone_updates[NUM_ANTS];
#pragma HLS ARRAY_PARTITION variable = pheromone_updates complete

      // Collect pheromone contributions from each ant
      for (int ant = 0; ant < NUM_ANTS; ant++) {
#pragma HLS UNROLL
        pheromone_updates[ant] = 1.0f / (1.0f + fitness[ant]);
      }

      // Apply evaporation
      float new_pheromone = pheromones[d] * (1.0f - EVAPORATION_RATE);

      // Reduce contributions from all ants
      for (int ant = 0; ant < NUM_ANTS; ant++) {
#pragma HLS UNROLL
        new_pheromone += pheromone_updates[ant];
      }

      // Update the pheromone
      pheromones[d] = new_pheromone;
    }
    DEBUG_PRINT("Iteration " << iter + 1 << "/" << ITERATIONS
                             << ", Best Fitness: " << best_fitness);
  }

  best_fitness_out.write(best_fitness);
  for (int i = 0; i < DIMENSIONS; i++) {
    best_solution_out.write(current_best_solution[i]);
  }
  DEBUG_PRINT("Best Solution: [");
  for (int i = 0; i < DIMENSIONS; i++) {
    DEBUG_PRINT(current_best_solution[i]);
    if (i < DIMENSIONS - 1)
      DEBUG_PRINT(", ");
  }
  DEBUG_PRINT("]");
}

void write_output_stream(hls::stream<float> &best_fitness_in,
                         hls::stream<float> &best_solution_in,
                         hls::stream<axis_out_t> &out_stream) {

  for (int i = 0; i < DIMENSIONS + 1; i++) {
    #pragma HLS PIPELINE II = 1

    axis_out_t word;

    // Read input value
    float value = (i == 0) ? best_fitness_in.read()
                            : best_solution_in.read();

    // Reinterpret float as uint32_t
    uint32_t data = *reinterpret_cast<uint32_t*>(&value);

    // Assign AXI4-Stream fields
    word.data = data;              // 32-bit payload
    word.keep = 0xF;               // All bytes valid (for 32-bit width)
    // word.strb = 0xF;               // All bytes valid (for 32-bit width)
    // word.user = 0;
    // word.id   = 0;
    // word.dest = 0;
    word.last = (i == DIMENSIONS); // Assert TLAST on final element

    // Debug output
    DEBUG_PRINT("OUT[" << i << "]: " << value << " (0x" << std::hex << data
                        << std::dec << ")");
    // Send over stream
    out_stream.write(word);
  }
}

void aco(hls::stream<axis_in_t> & in_stream,
           hls::stream<axis_out_t> & out_stream, int n) {
#pragma HLS INTERFACE axis register_mode = both port = in_stream
#pragma HLS INTERFACE axis register_mode = both port = out_stream
#pragma HLS INTERFACE s_axilite port = n
#pragma HLS INTERFACE s_axilite port = return
#pragma HLS DATAFLOW

  float ownship_x_arr[MAX_ENTRIES];
  float ownship_y_arr[MAX_ENTRIES];
  float measure_arr[MAX_ENTRIES];

  // Intermediate streams with FIFO depth
  hls::stream<float> best_fitness_stream("best_fitness_stream");
  hls::stream<float> best_solution_stream("best_solution_stream");

#pragma HLS STREAM variable = best_fitness_stream depth = 8
#pragma HLS STREAM variable = best_solution_stream depth = 8

  read_input_stream(in_stream, ownship_x_arr, ownship_y_arr, measure_arr, n);
  compute_aco_kernel(ownship_x_arr, ownship_y_arr, measure_arr, n,
                     best_fitness_stream, best_solution_stream);
  write_output_stream(best_fitness_stream, best_solution_stream, out_stream);
}