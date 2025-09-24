#!/bin/bash
set -Eeuo pipefail
trap 'echo "ERROR on line $LINENO"; exit 1' ERR

# --- Input validation ---
if [ $# -lt 4 ]; then
    echo "Usage: $0 <start_seed> <end_seed> <dimensions...> <input1.csv> [input2.csv ...] <output.csv>"
    exit 1
fi

start_seed=$1
end_seed=$2
shift 2  # remove first two arguments

# --- Last argument is output file ---
output_file=${!#}

# --- All remaining arguments except last are dimensions and input files ---
args=("${@:1:$#-1}")

# --- Extract dimensions (numeric args first) ---
dimensions=()
input_files=()
for arg in "${args[@]}"; do
    if [[ "$arg" =~ ^[0-9]+$ ]]; then
        dimensions+=("$arg")
    else
        input_files+=("$arg")
    fi
done

# Default dimensions if none provided
if [ ${#dimensions[@]} -eq 0 ]; then
    dimensions=(4 6 8)
fi

# --- Output CSV header ---
echo "INPUT_FILE,DIMENSIONS,SEED,SAMPLING_TIME,PARAM_SET,FITNESS,SOLUTION,ELAPSED_TIME" > "$output_file"

# --- Main loop ---
for input_file in "${input_files[@]}"; do
    echo "=== Processing INPUT_FILE=$input_file ==="

    # Compute SAMPLING_TIME
    if [[ -f "$input_file" ]]; then
        SAMPLING_TIME=$(awk -F',' '
            NR==2 {a=$1}
            NR==3 {b=$1}
            END{
                if (a=="" || b=="") {print "NaN"; exit}
                diff=b-a
                if (diff==diff) {printf "%.10g", diff} else {print "NaN"}
            }' "$input_file")
    else
        SAMPLING_TIME="NaN"
    fi
    if [[ "$SAMPLING_TIME" == "NaN" ]]; then
        SAMPLING_TIME=1
    fi

    # Set PARAM_SET
    if [[ "$SAMPLING_TIME" == "2" ]]; then
        PARAM_SET=0
    else
        PARAM_SET=1
    fi

    echo ">>> SAMPLING_TIME=$SAMPLING_TIME, PARAM_SET=$PARAM_SET"

    # Loop over selected dimensions
    for numvars in "${dimensions[@]}"; do
        echo "Compiling: DIMENSIONS=$numvars"
        g++ -DDIMENSIONS=$numvars -DSAMPLING_TIME=$SAMPLING_TIME -DPARAM_SET=$PARAM_SET -o aco aco.cpp

        for seed in $(seq "$start_seed" "$end_seed"); do
            echo "-> Running SEED=$seed"
            output=$(./aco "$seed" "$input_file")

            fitness=$(echo "$output" | grep "Best Fitness" | awk '{print $3}')
            solution=$(echo "$output" | grep "Best Solution" | cut -d ':' -f2- | xargs)
            solution="\"${solution}\""
            time=$(echo "$output" | grep "Elapsed Time" | awk '{print $3}')

            # Append to CSV
            echo "$(basename $input_file),$numvars,$seed,$SAMPLING_TIME,$PARAM_SET,$fitness,$solution,$time" >> "$output_file"
            echo "   Results -> FITNESS=$fitness, SOLUTION=$solution, TIME=${time}s"
        done
        echo ""
    done
done

echo ">>> All runs complete. Results saved to: $output_file"
