#!/bin/bash

# Check if all required arguments are provided
if [ $# -ne 4 ]; then
    echo "usage: $0 <start_seed> <end_seed> <input.csv> <output.csv>"
    exit 1
fi

# Output CSV file
output_file=$4

# Write CSV header
echo "Parameters,Seed,Best Fitness,Best Solution,Elapsed Time" > "$output_file"

# Loop over for different dimensions
for numvars in 4 6 8; do
    echo "Setting DIMENSIONS=$numvars"
# Loop over the seed range specified by the user
    for seed in $(seq $1 $2); do
        echo "Running seed $seed"
        output=$(python3 aco.py --seed=$seed --numvars=$numvars --csv=$3)

       # Extract values
        fitness=$(echo "$output" | grep "Best Fitness" | awk '{print $3}')
        solution=$(echo "$output" | grep "Best Solution" | cut -d ':' -f2- | xargs)
        solution="\"${solution}\""
        time=$(echo "$output" | grep "Elapsed Time" | awk '{print $3}')

        # Write to CSV
        echo "$numvars,$seed,$fitness,$solution,$time" >> "$output_file"
    done
done

echo "Results written to $output_file"
