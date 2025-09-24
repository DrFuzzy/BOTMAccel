#!/bin/bash
set -Eeuo pipefail
trap 'echo "ERROR on line $LINENO"; exit 1' ERR

# Usage:
# ./run_aco.sh <start_seed> <end_seed> <dimensions...> <input1.csv> [input2.csv ...] <output.csv>

if [ $# -lt 5 ]; then
    echo "Usage: $0 <start_seed> <end_seed> <dimensions...> <input1.csv> [input2.csv ...] <output.csv>"
    exit 1
fi

start_seed=$1
end_seed=$2
shift 2

# parse dimensions (one or many integer args)
dimensions=()
while [[ $# -gt 0 && $1 =~ ^[0-9]+$ ]]; do
    dimensions+=("$1")
    shift
done
if [ ${#dimensions[@]} -eq 0 ]; then
    echo "Error: at least one dimension must be provided (e.g. 4 6 8)"
    exit 1
fi

# remaining args: input files then last is output file
if [ $# -lt 2 ]; then
    echo "Error: provide at least one input file and one output file"
    exit 1
fi

output_file="${@: -1}"             # last arg
# get all but last as input files
input_files=("${@:1:$#-1}")

# --- Config & constants (keep these unchanged) ---
CFG_FILE="/home/delk/Desktop/BOTMA_FPGA/aco_maritime-unified_ide/hls_component/hls_config.cfg"
WORK_DIR="aco"

mkdir -p "$WORK_DIR"
mkdir -p logs

# --- Select mode (simulation or cosimulation) ---
echo "Select run mode:"
select mode in "simulation" "cosimulation"; do
  case $mode in
    simulation)
      RUN_MODE="--csim"
      break
      ;;
    cosimulation)
      RUN_MODE="--cosim"
      echo ">>> Cosimulation selected. Running HLS synthesis first..."
      v++ -c --mode hls --config "$CFG_FILE" --work_dir "$WORK_DIR"
      echo ">>> Synthesis complete. Proceeding with cosimulation runs."
      break
      ;;
    *) echo "Invalid choice";;
  esac
done

echo ">>> Running in $mode mode ($RUN_MODE)"
echo ""

# --- Helpers ---
upsert_define () {
  local macro="$1" value="$2" file="$3"
  if grep -q -- "-D${macro}=" "$file"; then
    sed -i -E "s/(-D${macro}=)[^[:space:]]+/\1${value}/g" "$file"
  elif grep -q -- "-DDIMENSIONS=" "$file"; then
    sed -i -E "0,/-DDIMENSIONS=[^[:space:]]+/s//& -D${macro}=${value}/" "$file"
  else
    echo "-D${macro}=${value}" >> "$file"
  fi
}

set_cfg_kv () {
  local key="$1" value="$2" file="$3"
  if grep -q -E "^[[:space:]]*${key}[[:space:]]*=" "$file"; then
    sed -i -E "s|^[[:space:]]*(${key})[[:space:]]*=.*$|\1=${value}|" "$file"
  else
    echo "${key}=${value}" >> "$file"
  fi
}

# robust extraction: everything after ':' or '=' and trimmed
extract_value_after_colon_or_eq () {
  awk -F'[:=]' '{ for(i=2;i<=NF;i++){printf "%s%s",$i,(i<NF?FS:"")} ; exit }' | sed -E 's/^[[:space:]]+|[[:space:]]+$//g'
}

# CSV quoting: double any internal double-quotes, then wrap in quotes
csv_quote() {
  local s="$1"
  s="${s//\"/\"\"}"
  printf '"%s"' "$s"
}

# --- CSV header (overwrite existing file) ---
echo "input_file,DIMENSIONS,SEED,SAMPLING_TIME,PARAM_SET,FITNESS,SOLUTION,ELAPSED_TIME" > "$output_file"

# --- Main loops ---
for input_file in "${input_files[@]}"; do
  # convert input_file to an absolute path (so config points correctly)
  if command -v realpath >/dev/null 2>&1; then
    CSV_PATH="$(realpath "$input_file")"
  elif command -v readlink >/dev/null 2>&1; then
    CSV_PATH="$(readlink -f "$input_file" || printf "%s" "$input_file")"
  else
    CSV_PATH="$input_file"
  fi

  echo "=== Scenario: $input_file ==="
  if [[ ! -f "$CSV_PATH" ]]; then
    echo ">>> Warning: input file not found at: $CSV_PATH -- sampling time will default to 1"
  fi

  # Make sure the hls_config points to the correct csv path
  set_cfg_kv "csim.argv"  "$CSV_PATH" "$CFG_FILE"
  set_cfg_kv "cosim.argv" "$CSV_PATH" "$CFG_FILE"

  # Compute SAMPLING_TIME from 1st column diff of 2nd and 3rd lines
  if [[ -f "$CSV_PATH" ]]; then
    SAMPLING_TIME=$(awk -F',' '
      NR==2 {a=$1}
      NR==3 {b=$1}
      END{
        if (a=="" || b=="") {print "NaN"; exit}
        diff=b-a
        if (diff==diff) {printf "%.10g", diff} else {print "NaN"}
      }' "$CSV_PATH")
  else
    SAMPLING_TIME="NaN"
  fi
  if [[ "$SAMPLING_TIME" == "NaN" ]]; then
    SAMPLING_TIME=1
  fi

  # PARAM_SET: if numeric SAMPLING_TIME equals 2 -> 0 else 1 (numeric compare)
  if awk -v v="$SAMPLING_TIME" 'BEGIN{ if (v+0==2) exit 0; else exit 1 }'; then
    PARAM_SET=0
  else
    PARAM_SET=1
  fi

  echo ">>> SAMPLING_TIME=${SAMPLING_TIME}, PARAM_SET=${PARAM_SET}"

  # push defines into config
  upsert_define "SAMPLING_TIME" "$SAMPLING_TIME" "$CFG_FILE"
  upsert_define "PARAM_SET"     "$PARAM_SET"     "$CFG_FILE"

  for DIM in "${dimensions[@]}"; do
    # ensure DIMENSIONS set in config
    if grep -q -- "-DDIMENSIONS=" "$CFG_FILE"; then
      sed -i -E "s/(-DDIMENSIONS=)[^[:space:]]+/\1${DIM}/g" "$CFG_FILE"
    else
      upsert_define "DIMENSIONS" "$DIM" "$CFG_FILE"
    fi

    for SEED in $(seq "$start_seed" "$end_seed"); do
      # set seed
      if grep -q -- "-DRANDOM_SEED=" "$CFG_FILE"; then
        sed -i -E "s/(-DRANDOM_SEED=)[^[:space:]]+/\1${SEED}/g" "$CFG_FILE"
      else
        upsert_define "RANDOM_SEED" "$SEED" "$CFG_FILE"
      fi

      echo "-> DIMENSIONS=$DIM, SEED=$SEED"

      # run vitis-run and capture stdout+stderr
      RUN_LOG="logs/$(basename "$input_file" | sed 's/[^a-zA-Z0-9_.-]/_/g')_D${DIM}_S${SEED}.log"
      OUTPUT="$(vitis-run --mode hls $RUN_MODE --config "$CFG_FILE" --work_dir "$WORK_DIR" 2>&1 || true)"
      printf '%s\n' "$OUTPUT" > "$RUN_LOG"

      # extract values robustly
      FITNESS=$(printf '%s\n' "$OUTPUT" | awk -F'[:=]' '/[Bb]est Fitness/ { val=""; for(i=2;i<=NF;i++){ val = val (i==2 ? $i : ":" $i) } gsub(/^[[:space:]]+|[[:space:]]+$/,"",val); print val; exit }')
      SOLUTION=$(printf '%s\n' "$OUTPUT" | awk -F'[:=]' '/[Bb]est Solution/ { val=""; for(i=2;i<=NF;i++){ val = val (i==2 ? $i : ":" $i) } gsub(/^[[:space:]]+|[[:space:]]+$/,"",val); print val; exit }')
      TIME=$(printf '%s\n' "$OUTPUT" | awk -F'[:=]' '/[Ee]lapsed Time/ { val=""; for(i=2;i<=NF;i++){ val = val (i==2 ? $i : ":" $i) } gsub(/^[[:space:]]+|[[:space:]]+$/,"",val); print val; exit }')

      # If values empty, note it and keep the log for debugging
      if [[ -z "$FITNESS" && -z "$SOLUTION" && -z "$TIME" ]]; then
        echo "   Warning: no 'Best Fitness/Best Solution/Elapsed Time' found in vitis-run output; saved raw output to $RUN_LOG"
      fi

      # CSV-safe quote the solution
      SAFE_SOLUTION=$(csv_quote "$SOLUTION")

      # Append the requested fields only
      echo "$(basename $input_file),$DIM,$SEED,$SAMPLING_TIME,$PARAM_SET,$FITNESS,$SAFE_SOLUTION,$TIME" >> "$output_file"

      # human-friendly terminal output
      printf "   Results -> FITNESS=%s, SOLUTION=%s, TIME=%ss\n" "${FITNESS:-}" "${SAFE_SOLUTION:-\"\"}" "${TIME:-}"
    done
  done
done

echo ">>> All runs complete. Results saved to: $output_file"
echo ">>> Per-run raw logs (stdout+stderr) are under the 'logs/' directory for debugging."
