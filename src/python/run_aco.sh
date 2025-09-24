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
shift 2  # Remove first two arguments

# --- Last argument is output CSV file ---
output_file=${!#}

# --- Remaining arguments (dimensions + input files) ---
args=("${@:1:$#-1}")

# Separate dimensions and input files
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
echo "INPUT_FILE,DIMENSIONS,SEED,PARAM_SET,SAMPLING_TIME,FITNESS,SOLUTION,ELAPSED_TIME" > "$output_file"

# --- Main loop ---
for input_file in "${input_files[@]}"; do
    echo "=== Processing INPUT_FILE=$input_file ==="

    # --- Compute SAMPLING_TIME from CSV ---
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

    # --- Set PARAM_SET depending on SAMPLING_TIME ---
    if [[ "$SAMPLING_TIME" == "2" ]]; then
        PARAM_SET=0
    else
        PARAM_SET=1
    fi

    echo ">>> SAMPLING_TIME=$SAMPLING_TIME, PARAM_SET=$PARAM_SET"

    for numvars in "${dimensions[@]}"; do
        echo "=== DIMENSIONS=$numvars ==="

        for seed in $(seq "$start_seed" "$end_seed"); do
            echo "-> SEED=$seed"

            # Run Python solver with calculated param_set
            output=$(python3 aco.py \
                --seed="$seed" \
                --numvars="$numvars" \
                --csv="$input_file" \
                --param_set="$PARAM_SET")

            # Extract values
            fitness=$(echo "$output" | grep "Best Fitness" | awk '{print $3}')
            solution=$(echo "$output" | grep "Best Solution" | cut -d ':' -f2- | xargs)
            solution="\"${solution}\""
            time=$(echo "$output" | grep "Elapsed Time" | awk '{print $3}')

            # Append results
            echo "$(basename $input_file),$numvars,$seed,$PARAM_SET,$SAMPLING_TIME,$fitness,$solution,$time" >> "$output_file"

            # Print formatted results
            echo "   Results -> FITNESS=$fitness, SOLUTION=$solution, TIME=${time}s"
        done
        echo ""
    done
done

echo ">>> All runs complete. Results saved to: $output_file"
