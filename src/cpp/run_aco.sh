#!/bin/bash

# Check if two arguments are provided (start and end of the seed range)
if [ $# -ne 4 ]; then
    echo "usage: $0 <start_seed> <end_seed> <input.csv> <output.csv>"
    exit 1
fi

# Output CSV file
output_file=$4

# Write CSV header
echo "Seed,Best Fitness,Elapsed Time" > "$output_file"

# Loop over the seed range specified by the user
for seed in $(seq $1 $2); do
    echo "Running seed $seed..."
    output=$(./aco $seed $3)

    # Extract values
    fitness=$(echo "$output" | grep "Best Fitness" | awk '{print $3}')
    time=$(echo "$output" | grep "Elapsed Time" | awk '{print $3}')

    # Write to CSV
    echo "$seed,$fitness,$time" >> "$output_file"
done

echo "Results written to $output_file"
