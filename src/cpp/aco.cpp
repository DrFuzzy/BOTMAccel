#include "aco.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <limits>
#include <random>
#include <chrono>

using namespace std;

// Parameter ranges (same as in Python code)
const float parameter_ranges[DIMENSIONS][2] = {
    {0, 50000}, // Range for theta[0]
    {0, 50000}, // Range for theta[1]
    {-10, 10},  // Range for theta[2]
    {-10, 10},  // Range for theta[3]
    {0, 0},     // Range for theta[4]
    {0, 0},     // Range for theta[5]
};

// Declare global arrays for data input
float ownship_x[MAX_ENTRIES];
float ownship_y[MAX_ENTRIES];
float measure[MAX_ENTRIES];
float timeframe[MAX_ENTRIES];
int data_size = 0;

// Random number generator (for simplicity, linear congruential generator)
unsigned int random_state; // globally assigned in main()
float random_float() {
    random_state = random_state * 1664525 + 1013904223;
    return (random_state % 1000) / 1000.0f; // Generate float in [0, 1)
}

// // 32-bit PCG-like generator (HLS-friendly)
// //static uint64_t rng_state = 0x853c49e6748fea9bULL;
// static uint64_t rng_state = RANDOM_SEED;
// static const uint64_t rng_mult = 6364136223846793005ULL;
// static const uint64_t rng_inc  = 1442695040888963407ULL;

// // Output is truncated to 24-bit mantissa-like float precision.
// // You can change rng_state to accept a seed for reproducibility.
// float random_float() {
//     rng_state = rng_state * rng_mult + rng_inc;
//     uint32_t xorshifted = ((rng_state >> 18u) ^ rng_state) >> 27u;
//     uint32_t rot = rng_state >> 59u;
//     uint32_t result = (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
//     return (result & 0xFFFFFF) / 16777216.0f; // Convert to [0,1)
// }

// //LFG (Lagged Fibonacci Generator) - HLS-friendly
// static uint64_t lfg_state[2] = {RANDOM_SEED, RANDOM_SEED + 1}; // Two 64-bit states 
// static const uint64_t lfg_m = 24; // Lag 1 
// static const uint64_t lfg_n = 55; // Lag 2

// float random_float() {
//     uint64_t next = (lfg_state[0] + lfg_state[1]) % (1ULL << 64); // Fibonacci sum 
//     lfg_state[0] = lfg_state[1]; // Shift states lfg_state[1] = next; 
//     // Update new state
//     return (next * 0.5 / (1ULL << 64)); // Normalize to [0, 1)
// }

// // Single-state Xorshift RNG (HLS-friendly)
// static unsigned long xorshift_state = RANDOM_SEED;  // Single 32-bit or
// // 64-bit state depending on system 
// static const unsigned long xorshift_mult = 6364136223846793005UL;  // Multiplier for RNG 
// static const unsigned long xorshift_add = 1442695040888963407UL;  // Golden ratio for better randomness

// // Updated random_float function to return a float value in the range [0, 1)
// float random_float() {
//     // Update state with the XOR shift algorithm
//     xorshift_state = xorshift_state * xorshift_mult + xorshift_add;

//     // XOR shift to generate the result (use 32 bits for randomness)
//     unsigned int result = (xorshift_state >> 18) ^ static_cast<unsigned
//     int>(xorshift_state);

//     // Normalize the result to the range [0, 1)
//     return static_cast<float>(result) / static_cast<float>(0xFFFFFFFF);
// }

// // Seed and RNG setup
// mt19937 rng(RANDOM_SEED);
// // Function to return a random float in [0, 1)
// double random_float() {
//     uniform_real_distribution<double> dist(0.0, 1.0);
//     return dist(rng);
// }

// Function to calculate the objective function
float objective_function(const float theta[DIMENSIONS], const float ownship_x[],
                         const float ownship_y[], const float measure[],
                         int n) {
    float sum_squared_diff = 0.0f;
    unsigned int timeframe = 0;

    for (int i = 0; i < n; i++) {

        // Advance timeframe
        timeframe += 2; // sampling rate = 2

        // Trajectory of the target based on the parameter vector theta
        float x_t = theta[0] + timeframe * theta[2] +
                    (timeframe * timeframe * theta[4]) / 2.0f;
        float y_t = theta[1] + timeframe * theta[3] +
                    (timeframe * timeframe * theta[5]) / 2.0f;

        // Calculate h (angle)
        float h = atan2f(y_t - ownship_y[i], x_t - ownship_x[i]);

        // Accumulate squared difference
        sum_squared_diff += (measure[i] - h) * (measure[i] - h);
    }

    return sum_squared_diff;
}

void aco(const float ownship_x[], const float ownship_y[],
         const float measure[], float &best_fitness,
         float best_solution[DIMENSIONS]) {

    // Initialise pheromones and best solution
    float pheromones[DIMENSIONS] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    best_fitness = numeric_limits<float>::max();

    // ACO algorithm
    for (int iter = 0; iter < ITERATIONS; iter++) {
        float ants[NUM_ANTS][DIMENSIONS]; // Ants' solutions
        float fitness[NUM_ANTS];          // Fitness of each ant

        // Generate solutions and evaluate fitness
        for (int ant = 0; ant < NUM_ANTS; ant++) {
            for (int d = 0; d < DIMENSIONS; d++) {
                // Generate a random value in the parameter range
                float random_value = random_float();
                ants[ant][d] = parameter_ranges[d][0] +
                               random_value * (parameter_ranges[d][1] -
                                               parameter_ranges[d][0]);
            }

            // Evaluate fitness
            fitness[ant] = objective_function(ants[ant], ownship_x, ownship_y,
                                              measure, MAX_ENTRIES);

            // Update best solution if needed
            if (fitness[ant] < best_fitness) {
                best_fitness = fitness[ant];
                for (int i = 0; i < DIMENSIONS; i++) {
                    best_solution[i] = ants[ant][i];
                }
            }
        }

        // Update pheromones
        for (int d = 0; d < DIMENSIONS; d++) {
            pheromones[d] *= (1.0f - EVAPORATION_RATE); // Evaporation
            for (int ant = 0; ant < NUM_ANTS; ant++) {
                pheromones[d] +=
                    1.0f / (1.0f + fitness[ant]); // Deposit pheromones
            }
        }
        // Debug output for monitoring
        // printf("Iteration %d/%d: Best Fitness = %.6f\n", iter + 1,
        // ITERATIONS, best_fitness);
    }
}

void load_data(const char *file_path) {
    ifstream file(file_path);

    if (!file || !file.is_open()) {
        cerr << "Error: Unable to open input file: \n" << file_path;
        return;
    }

    string line;
    if (getline(file, line)) {
        cout << "Skipping header: " << line;
    }

    int index = 0;
    while (getline(file, line)) {
        stringstream ss(line);
        char comma;

        // Parse four floats from CSV line
        ss >> timeframe[index] >> comma >> ownship_x[index] >> comma >>
            ownship_y[index] >> comma >> measure[index];

        DEBUG_PRINT(index << ", " << "timeframe = " << timeframe[index] << ", "
         << "ownship_x = " << ownship_x[index] << ", "
         << "ownship_y = " << ownship_y[index] << ", "
         << "measure = " << measure[index]);

        index++;
    }

    data_size = index;
    file.close();
    cout << "\nData successfully loaded. Total entries: " << data_size << "\n";
}

int main(int argc, char* argv[]) {
    if (argc != 3) {  // Expect exactly two arguments
        cerr << "usage: %s <seed> <filename.csv>\n" << argv[0];
        return 1;
    }

    char *endptr;
    long seed = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0') {
        cerr << "Invalid integer: %s\n" << argv[1];
        return 1;
    }

    random_state = (int)seed;

    size_t len = strlen(argv[2]);
    if (len < 4 || strcmp(argv[2] + len - 4, ".csv") != 0) {
        cerr << "Input file must end in .csv\n";
        return 1;
    }
    
    // Load data
    load_data(argv[2]);

    // Ensure that data_size is valid
    if (data_size <= 0) {
        cerr << "No data loaded. Exiting..." << endl;
        return 1;
    }

    // Allocate arrays for best_solution and best_fitness (outputs)
    float best_solution[DIMENSIONS];
    float best_fitness;

    // Start timing
    auto start_time = chrono::high_resolution_clock::now();

    // Run ACO routine
    cout << "Running ACO minimisation...\n";
    aco(ownship_x, ownship_y, measure, best_fitness, best_solution);

    // Print results
    cout << "Best Solution: ";
    for (int i = 0; i < DIMENSIONS; i++) {
        cout << best_solution[i] << " ";
    }
    cout << "\nBest Fitness: " << best_fitness << "\n";
    
    // Stop timing
    auto end_time = chrono::high_resolution_clock::now();
    auto elapsed = chrono::duration<double>(end_time - start_time).count();
    cout << "Elapsed Time: " << elapsed << " seconds" << endl;

    return 0;
}
