
# BOTMA (Bearings-Only Target Motion Analysis) Accelerator (BOTMAccel)

This repository contains an implementation for an FPGA-based SoC Accelerator for Ant Colony Optimization in Bearings-Only Target Motion Analysis. Code implementations in Python, MATLAB, C++, and HLS are provided. A `run_aco.sh` bash script is included to run the algorithms in Python and C++ with multiple random seeds.

## Citation

If you find this code useful or use it in your research, please cite the following paper, where this codebase was used to generate the experimental results:

```bibtex
@ARTICLE{11222596,
  author  = {Deliparaschos, Kyriakos M. and Oliva, Gabriele},
  journal = {IEEE Access},
  title   = {FPGA-Accelerated ϵ-Greedy Ant Colony Optimisation for Maritime Bearings-Only Nonlinear Target Motion Analysis},
  year    = {2025},
  volume  = {13},
  pages   = {190027--190039},
  doi     = {10.1109/ACCESS.2025.3626686}
}
```

## Prerequisites

* **Python 3.x** (for the Python implementation)
* **C++ compiler** (e.g., `g++`, for the C++ implementation)
* **Vitis 2025.1** (for the HLS flow)
* **Vivado 2025.1** (for the FPGA implementation)
* **Bash shell** (for executing the `.sh` script)
* **PYNQ-Z1 board** (for testing the accelerator)

## Project Folder Structure

📂 **BOTMAccel/**  
├── 📄 **README.md**  
├── 📄 **LICENSE**  

📂 **src/**  
├── 📂 **cpp/**                 - C++ implementation  
├── 📂 **python/**              - Python implementation  
├── 📂 **hls/**                 - Vitis HLS C++ implementation  
├── 📂 **matlab/**              - MATLAB helper scripts  
└── 📂 **pynq/**                - PYNQ notebooks  

📂 **vivado/**                  - Vivado integration project  
├── 📂 **bd/**                  - Block diagram files  
└── 📄 **Makefile**  

📂 **data/**                    - Input/output data files  
├── 📂 **input/**  
└── 📂 **output/**  

## Setup Instructions

After cloning the repository, run the `setup_data.sh` script to download and extract the dataset required for this project.

```bash
./setup_data.sh
```

This will download the "Ushant AIS" dataset, a collection of vessel trajectories recorded using AIS in the Ushant region from the official source provided in the Ushant AIS GitHub repository.

The data will be extracted into the `./data` directory, and the original ZIP file will be removed after extraction.

## Python Implementation

### Running the Python Implementation

To run the Python ACO script, use the following command:

```bash
python3 aco.py --seed 12345 --csv example1.csv
```

**Usage:**

```
aco.py [-h] [--seed SEED] --csv CSV_FILE
```

## C++ Implementation

### Running the C++ Implementation

To run the C++ ACO program, compile it first (if not already compiled), then run:

```bash
./aco 12345 example1.csv
```

**Usage:**

```
./aco <seed> <filename.csv>
```

## HLS Implementation

### Running the HLS Implementation

To run the HLS ACO program, use the provided makefile to execute either simulation or cosimulation. First, configure the necessary arguments and flags (such as seed, input file, and other parameters) in the `hls_config.cfg` file.

To run **simulation**, use:

```bash
make csim
```

To run **cosimulation**, use:

```bash
make cosim
```

### Usage:

- Configure the required arguments and flags in the `hls_config.cfg` file.
- Alternatively, you can use the **Vitis IDE**. First, create an HLS component in Vitis IDE, then build the project from there.

This allows you to run the HLS implementation either via the makefile or through the Vitis IDE.

## Bash Script

### Running ACO with `run_aco.sh`

This script can be used to run the ACO implementations across a range of seeds, dimensions, and input files. The script is available in the following directories for running batch results with different implementations:

- **Python**: `/src/python/run_aco.sh`
- **C++**: `/src/cpp/run_aco.sh`
- **HLS**: `/src/hls/run_aco.sh`

These scripts are tailored to execute the corresponding implementation (Python, C++, or HLS) and can be used for batch processing of results.

The HLS script prompts the user to choose between **simulation** or **cosimulation** before running the batch process.

**Usage:**

```
./run_aco.sh <start_seed> <end_seed> <dimensions...> <input1.csv> [input2.csv ...] <output.csv>
```

**Examples:**

```bash
./run_aco.sh 1 1 6 ../../data/real_world_example.csv ../../data/output/cpp_results.csv
./run_aco.sh 1 10 4 6 8 ../../data/input/output_linear.csv ../../data/input/output_polynomial.csv ../../data/input/output_uniformly_accelerated.csv ../../data/real_world_example.csv ../../data/output/cpp_results.csv
./run_aco.sh 1 10 4 6 8 ../../data/input/output_linear.csv ../../data/input/output_polynomial.csv ../../data/input/output_uniformly_accelerated.csv ../../data/real_world_example.csv ../../data/output/python_results.csv
./run_aco.sh 1 10 4 6 8 ../../data/input/output_linear.csv ../../data/input/output_polynomial.csv ../../data/input/output_uniformly_accelerated.csv ../../data/real_world_example.csv ../../data/output/hls_sim_results.csv
./run_aco.sh 1 10 4 6 8 ../../data/input/output_linear.csv ../../data/input/output_polynomial.csv ../../data/input/output_uniformly_accelerated.csv ../../data/real_world_example.csv ../../data/output/hls_cosim_results.csv
```

## Creating the Vivado Project

To create the Vivado project, navigate to the Vivado folder and execute the following command:

```bash
vivado -source project.tcl
```

After the project is created, run the implementation and generate the bitstream. Then, go to **File** > **Export** > **Export Hardware**, and make sure to select include bitstream.

## Running the PYNQ Overlay for the BOTMA Accelerator

### 1. Copy Required Files to the FPGA Board

Copy the following files from the Vivado project to the FPGA board’s SD card:

- **.bit** (e.g. `design_3_wrapper.bit`)
- **.hwa** (e.g. `design_3_wrapper.hwa`)
- **.tcl** (e.g. `design_3_wrapper.tcl`)

### 2. Copy Jupyter Notebooks to the SD Card

Copy the Jupyter Notebook files from `/src/pynq/notebooks` to the `notebooks` folder on the SD card.

Then, open a browser and load the PYNQ overlay to interact with the BOTMA accelerator.