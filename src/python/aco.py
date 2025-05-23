import numpy as np
import pandas as pd
import time
import argparse

# Constants
NUM_ANTS = 100 # Number of ants
DIMENSIONS = 6 # Number of parameters
ITERATIONS = 1000 # Maximum number of iterations
EVAPORATION_RATE = 0.1 # Pheromone evaporation rate

# Define the parser
parser = argparse.ArgumentParser(description='ACO BOTMA')

# Declare arguments
parser.add_argument('--debug', action="store", dest='DEBUG', type=int, default=0)
parser.add_argument('--seed', action="store", dest='seed', default=0)
parser.add_argument('--numvars', action="store", dest='numvars', type=int, choices=[4, 6, 8],
                    required=True, help="Number of variables: must be 4, 6, or 8")
parser.add_argument('--csv', action="store", dest='csv_file', required=True, help="Path to the input CSV file")

# Parse the command line arguments and store the values in the `args` variable
args = parser.parse_args()

# Global variables to store the data from .csv
ownship_x = None
ownship_y = None
measure = None
timeframe = None

# Toggle debugging based on an environment variable or a constant
DEBUG = args.DEBUG

def debug_print(*args):
    if DEBUG:
        print("[DEBUG]", *args)

# Objective function implementation
def objective_function(theta):
    global ownship_x, ownship_y, measure, timeframe, sigma
    n = len(ownship_x)
    sum_squared_diff = 0.0

    for i in range(n):
        # Compute target trajectory using a polynomial expansion vector theta
        pow_ = 1
        fact = 1
        x_t = 0.0
        y_t = 0.0
        j = 0
        a = 1

        while j < int(args.numvars) :
            gamma = pow_ / fact
            x_t += theta[j] * gamma
            j += 1
            y_t += theta[j] * gamma
            j += 1
            pow_ *= timeframe[i]
            fact *= a
            a += 1
            
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
    debug_print("Timeframe:", timeframe)
    debug_print("Ownship X:", ownship_x)
    debug_print("Ownship Y:", ownship_y)
    debug_print("Measure:", measure)

# Optimised ACO routine with parameter ranges
def aco():
    global ownship_x, ownship_y, measure, timeframe, sigma

    # Parameter ranges
    parameter_ranges = [
        (20000, 40000),  # Range for theta[0]
        (20000, 40000),  # Range for theta[1]
        (5, 10),    # Range for theta[2]
        (5, 10),    # Range for theta[3]
        (-0.01, 0.01),    # Range for theta[4]
        (-0.01, 0.01),    # Range for theta[5]
        (-0.0001, 0.0001),    # Range for theta[6]
        (-0.0001, 0.0001),    # Range for theta[7]
    ]

    # Initialise pheromones and best solution
    pheromones = np.ones(int(args.numvars))
    best_solution = np.zeros(int(args.numvars))
    best_fitness = float('inf')

    # RNG for reproducibility
    rng = np.random.default_rng(int(args.seed))

    for iter in range(ITERATIONS):
        ants = np.zeros((NUM_ANTS, int(args.numvars)))
        fitness = np.zeros(NUM_ANTS)

        # Generate solutions and evaluate fitness
        for ant in range(NUM_ANTS):
            for d in range(int(args.numvars)):
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
        for d in range(int(args.numvars)):
            pheromones[d] *= (1.0 - EVAPORATION_RATE)  # Evaporation
            for ant in range(NUM_ANTS):
                pheromones[d] += 1.0 / (1.0 + fitness[ant])  # Deposit pheromones

        # Debug output for monitoring
        debug_print(f"Iteration {iter + 1}/{ITERATIONS}: Best Fitness = {best_fitness:.6f}")

    return best_solution, best_fitness

# Load data
load_data(args.csv_file)

# Run ACO routine
start_time = time.time()
best_solution, best_fitness = aco()
end_time = time.time()

# Print results
print(f"Best Solution: {' '.join(map(str, best_solution))}") # Remove []
print(f"Best Fitness: {best_fitness:.6f}")
print(f"Elapsed Time: {end_time - start_time:.2f} seconds")
