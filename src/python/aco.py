import numpy as np
import pandas as pd
import time
import argparse

# Constants
NUM_ANTS = 20  # Number of ants
DIMENSIONS = 6  # Number of parameters
ITERATIONS = 1000  # Maximum number of iterations
EVAPORATION_RATE = 0.1  # Pheromone evaporation rate

# Define the parser
parser = argparse.ArgumentParser(description='ACO BOTMA')

# Declare arguments
parser.add_argument('--debug', action="store", dest='DEBUG', type=int, default=0)
parser.add_argument('--seed', action="store", dest='seed', default=0)
parser.add_argument('--numvars', action="store", dest='numvars', type=int, choices=[4, 6, 8],
                    required=True, help="Number of variables: must be 4, 6, or 8")
parser.add_argument('--csv', action="store", dest='csv_file', required=True, help="Path to the input CSV file")
parser.add_argument('--param_set', action="store", dest='param_set', type=int, choices=[0, 1], default=0,
                    help="Choose parameter set: 0 = near-linear, 1 = polynomial")

# Parse the command line arguments and store the values in the `args` variable
args = parser.parse_args()

# Global variables to store the data from .csv
ownship_x = None
ownship_y = None
measure = None
timeframe = None

# Toggle debugging based on argument
DEBUG = args.DEBUG

def debug_print(*msg):
    if DEBUG:
        print("[DEBUG]", *msg)

# Objective function implementation
def objective_function(theta):
    global ownship_x, ownship_y, measure, timeframe
    n = len(ownship_x)
    sum_squared_diff = 0.0

    for i in range(n):
        pow_ = 1
        fact = 1
        x_t = 0.0
        y_t = 0.0
        j = 0
        a = 1

        while j < int(args.numvars):
            gamma = pow_ / fact
            x_t += theta[j] * gamma
            j += 1
            y_t += theta[j] * gamma
            j += 1
            pow_ *= timeframe[i]
            fact *= a
            a += 1

        h = np.arctan2((y_t - ownship_y[i]), (x_t - ownship_x[i]))
        sum_squared_diff += (measure[i] - h) ** 2

    return sum_squared_diff

# Function to load data from .csv
def load_data(file_path):
    global ownship_x, ownship_y, measure, timeframe
    data = pd.read_csv(file_path, header=0, names=["timeframe", "ownship_x", "ownship_y", "measure"])
    timeframe = data["timeframe"].astype(float).values
    ownship_x = data["ownship_x"].astype(float).values
    ownship_y = data["ownship_y"].astype(float).values
    measure = data["measure"].astype(float).values
    print(f"Data successfully loaded from {file_path}")

# Optimised ACO routine with parameter ranges
def aco():
    # Select parameter ranges based on param_set
    if args.param_set == 0:
        parameter_ranges = [
            (20000, 40000),      # theta[0]
            (20000, 40000),      # theta[1]
            (5, 10),             # theta[2]
            (5, 10),             # theta[3]
            (-0.01, 0.01),       # theta[4]
            (-0.01, 0.01),       # theta[5]
            (-0.0001, 0.0001),   # theta[6]
            (-0.0001, 0.0001),   # theta[7]
        ]
    else:
        parameter_ranges = [
            (-500000, -200000),   # theta[0]
            (2000000, 5000000),   # theta[1]
            (0, 10),              # theta[2]
            (0, 10),              # theta[3]
            (0, 0.001),           # theta[4]
            (-0.001, 0),          # theta[5]
            (-0.00001, 0.00001),  # theta[6]
            (-0.00001, 0.00001),  # theta[7]
        ]

    pheromones = np.ones(int(args.numvars))
    best_solution = np.zeros(int(args.numvars))
    best_fitness = float('inf')

    rng = np.random.default_rng(int(args.seed))

    for iter in range(ITERATIONS):
        ants = np.zeros((NUM_ANTS, int(args.numvars)))
        fitness = np.zeros(NUM_ANTS)

        for ant in range(NUM_ANTS):
            for d in range(int(args.numvars)):
                lower_bound, upper_bound = parameter_ranges[d]
                random_value = rng.random()
                ants[ant][d] = lower_bound + random_value * (upper_bound - lower_bound)

                # Bias randomization using pheromones
                bias = pheromones[d]
                biased_rand = pow(random_value, 1.0 / (1.0 + bias))
                if rng.random() <= 0.2:  # epsilon-greedy
                    biased_rand = random_value
                ants[ant][d] = lower_bound + biased_rand * (upper_bound - lower_bound)

            fitness[ant] = objective_function(ants[ant])
            if fitness[ant] < best_fitness:
                best_fitness = fitness[ant]
                best_solution = ants[ant].copy()

        for d in range(int(args.numvars)):
            pheromones[d] *= (1.0 - EVAPORATION_RATE)
            for ant in range(NUM_ANTS):
                pheromones[d] += 1.0 / (1.0 + fitness[ant])

        debug_print(f"Iteration {iter + 1}/{ITERATIONS}: Best Fitness = {best_fitness:.6f}")

    return best_solution, best_fitness

# Load data
load_data(args.csv_file)

# Run ACO routine
start_time = time.time()
best_solution, best_fitness = aco()
end_time = time.time()

# Print results
print(f"Best Solution: {' '.join(map(str, best_solution))}")
print(f"Best Fitness: {best_fitness:.6f}")
print(f"Elapsed Time: {end_time - start_time:.2f} seconds")
