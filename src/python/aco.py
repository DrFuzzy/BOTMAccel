import numpy as np
import pandas as pd
import time
import argparse

# Define the parser
parser = argparse.ArgumentParser(description='Short sample app')

# Declare arguments
parser.add_argument('--seed', action="store", dest='seed', default=0)
parser.add_argument('--csv', action="store", dest='csv_file', required=True, help="Path to the input CSV file")

# Now, parse the command line arguments and store the 
# values in the `args` variable
args = parser.parse_args()

# Individual arguments can be accessed as attributes...
#print(args.seed)

# Global variables to store the data from .csv
ownship_x = None
ownship_y = None
measure = None
timeframe = None

# Objective function implementation
def objective_function(theta):
    global ownship_x, ownship_y, measure, timeframe, sigma
    n = len(ownship_x)
    sum_squared_diff = 0.0

    for i in range(n):
        # Trajectory of the target based on the parameter vector theta
        x_t = theta[0] + timeframe[i] * theta[2] + (timeframe[i] ** 2 * theta[4]) / 2.0
        y_t = theta[1] + timeframe[i] * theta[3] + (timeframe[i] ** 2 * theta[5]) / 2.0

        # Calculate h
        h = np.arctan2((y_t - ownship_y[i]), (x_t - ownship_x[i]))

        # Accumulate squared difference
        sum_squared_diff += (measure[i] - h) ** 2

    # Final computation of the objective function
    f = sum_squared_diff
    return f

# Function to load data from .csv
def load_data(file_path):
    global ownship_x, ownship_y, measure, timeframe
    data = pd.read_csv(file_path, header=0, names=["timeframe", "ownship_x", "ownship_y", "measure"])
    timeframe = data["timeframe"].astype(float).values
    ownship_x = data["ownship_x"].astype(float).values
    ownship_y = data["ownship_y"].astype(float).values
    measure = data["measure"].astype(float).values
    print("Data successfully loaded:")
    #print("Timeframe:", timeframe)
    #print("Ownship X:", ownship_x)
    #print("Ownship Y:", ownship_y)
    #print("Measure:", measure)

# Optimized ACO routine with parameter ranges
def aco_minimization():
    global ownship_x, ownship_y, measure, timeframe, sigma

    # Constants
    NUM_ANTS = 100
    DIMENSIONS = 6
    ITERATIONS = 2000
    EVAPORATION_RATE = 0.1

    # Parameter ranges
    parameter_ranges = [
        (0, 50000),  # Range for theta[0]
        (0, 50000),  # Range for theta[1]
        (-10, 10),    # Range for theta[2]
        (-10, 10),    # Range for theta[3]
        (0, 0),    # Range for theta[4]
        (0, 0),    # Range for theta[5]
    ]

    # Initialize pheromones and best solution
    pheromones = np.ones(DIMENSIONS)
    best_solution = np.zeros(DIMENSIONS)
    best_fitness = float('inf')

    # RNG for reproducibility
    rng = np.random.default_rng(int(args.seed))

    for iter in range(ITERATIONS):
        ants = np.zeros((NUM_ANTS, DIMENSIONS))
        fitness = np.zeros(NUM_ANTS)

        # Generate solutions and evaluate fitness
        for ant in range(NUM_ANTS):
            for d in range(DIMENSIONS):
                lower_bound, upper_bound = parameter_ranges[d]
                random_value = rng.random()  # Generate a random number in [0, 1]
                ants[ant][d] = lower_bound + random_value * (upper_bound - lower_bound)  # Scale to parameter range

            # Evaluate fitness
            fitness[ant] = objective_function(ants[ant])

            # Update best solution if needed
            if fitness[ant] < best_fitness:
                best_fitness = fitness[ant]
                best_solution = ants[ant].copy()

        # Update pheromones
        for d in range(DIMENSIONS):
            pheromones[d] *= (1.0 - EVAPORATION_RATE)  # Evaporation
            for ant in range(NUM_ANTS):
                pheromones[d] += 1.0 / (1.0 + fitness[ant])  # Deposit pheromones

        # Debug output for monitoring
        #print(f"Iteration {iter + 1}/{ITERATIONS}: Best Fitness = {best_fitness:.6f}")

    return best_solution, best_fitness

# Load data
load_data(args.csv_file)

# Run ACO routine
start_time = time.time()
best_solution, best_fitness = aco_minimization()
end_time = time.time()

# Print results
print(f"Best Solution: {best_solution}")
print(f"Best Fitness: {best_fitness:.6f}")
print(f"Elapsed Time: {end_time - start_time:.2f} seconds")
