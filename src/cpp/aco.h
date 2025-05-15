#ifndef ACO_H
#define ACO_H

#include <iostream>

#ifdef DEBUG
#define DEBUG_PRINT(x) std::cout << "[DEBUG] " << x << std::endl
#else
#define DEBUG_PRINT(x)
#endif

// ACO parameters
#define NUM_ANTS 100         // Number of ants
#define DIMENSIONS 6         // Number of dimensions (6 parameters in total)
#define ITERATIONS 2000      // Maximum number of iterations
#define EVAPORATION_RATE 0.1 // Pheromone evaporation rate
#define MAX_ENTRIES 1801     // Adjust this as per your actual data size

void aco(const float ownship_x[], const float ownship_y[],
         const float measure[], const float timeframe[], float &best_fitness,
         float best_solution[DIMENSIONS], int n);

#endif // ACO_H
