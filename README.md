# BOTMA (Bearings-Only Target Motion Analysis) Accelerator 

This repository contains an implementation for an FPGA-Based SoC Accelerator for Ant Colony Optimisation in Bearings-Only Target Motion Analysis. Different code implementations in Python, MATLAB, C++, and HLS are provided. A `run_aco.sh` bash script is included to run the algorithms in Python and C++ with multiple random seeds.

## Prerequisites

* **Python 3.x** (for the Python implementation)
* **C++ compiler** (e.g., `g++`, for the C++ implementation)
* **Bash shell** (for executing the `.sh` script)

## Setup Instructions

After cloning the repository, you must run the `setup_data.sh` script to download and extract the dataset required for this project.

```bash
./setup_data.sh
```

This will download the "Ushant AIS" dataset which is a collection of vessel trajectories recorded using AIS in the Ushant region from the official source provided in the Ushant AIS GitHub repository.

The data will be extracted into the ./data directory, and the original ZIP file will be removed after extraction.

## Python Version

### Running the Python Implementation

To run the Python ACO script, use the following command:

```bash
python3 aco.py --seed 12345 --csv example1.csv
```

**Usage:**

```
aco.py [-h] [--seed SEED] --csv CSV_FILE
```

## C++ Version

### Running the C++ Implementation

To run the C++ ACO program, compile it first (if not already compiled), then run:

```bash
./aco 12345 example1.csv
```

**Usage:**

```
./aco <seed> <filename.csv>
```

## Bash Script

### Running ACO with `run_aco.sh`

This script can be used to run the ACO implementations across a range of seeds.

**Usage:**

```
./run_aco.sh <start_seed> <end_seed> <input.csv> <output.csv>
```

**Examples:**

```bash
./run_aco.sh 1 100 example1.csv aco_python_results.csv
./run_aco.sh 1 100 example1.csv aco_cpp_results.csv
```

