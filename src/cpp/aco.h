#ifndef ACO_H
#define ACO_H

#include <iostream>

#ifdef DEBUG
#define DEBUG_PRINT(x) std::cout << "[DEBUG] " << x << std::endl
#else
#define DEBUG_PRINT(x)
#endif

// Constants
#define NUM_ANTS 100 // Number of ants
#ifndef DIMENSIONS
#warning "No DIMENSIONS set, defaulting to DIMENSIONS=6"
#define DIMENSIONS 6 // Number of parameters
#endif

#if DIMENSIONS != 4 && DIMENSIONS != 6 && DIMENSIONS != 8
#error                                                                         \
    "Invalid DIMENSIONS value. DIMENSIONS must be defined as 4, 6, or 8 (e.g., -DDIMENSIONS=6)"
#endif

#define ITERATIONS 1000      // Maximum number of iterations
#define EVAPORATION_RATE 0.1 // Pheromone evaporation rate
#define MAX_ENTRIES 1801     // Adjust this as per your actual data size
#define SAMPLING_TIME 2      // Sampling time

void aco(const float ownship_x[], const float ownship_y[],
         const float measure[], const float timeframe[], float &best_fitness,
         float best_solution[DIMENSIONS], int n);

#endif // ACO_H
