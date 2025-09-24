#include "aco.h"
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

// AXI Stream data type (32-bit for output, 96-bit for input)
typedef ap_axiu<96, 1, 1, 1> axis_in_t;
typedef ap_axiu<32, 1, 1, 1> axis_out_t;

// Load input data from CSV file into in_stream (packed)
void load_data(const char *file_path, hls::stream<axis_in_t> &in_stream) {
  ifstream infile(file_path);
  if (!infile) {
    cerr << "Error: Unable to open input file: " << file_path << endl;
    return;
  }

  string line;

  // Skip header
  if (getline(infile, line)) {
    cout << "Skipping header: " << line << endl;
  }

  // Count valid lines (to determine tlast position)
  int num_lines = 0;
  while (getline(infile, line)) {
    if (!line.empty()) num_lines++;
  }

  // Rewind file
  infile.clear();
  infile.seekg(0, ios::beg);
  getline(infile, line); // skip header again

  int i = 0;
  while (getline(infile, line)) {
    stringstream ss(line);
    float ownship_x, ownship_y, measure, timeframe;
    char comma;

    ss >> timeframe >> comma >> ownship_x >> comma >> ownship_y >> comma >> measure;

    union { float f; uint32_t i; } ux, uy, um;
    ux.f = ownship_x;
    uy.f = ownship_y;
    um.f = measure;

    axis_in_t input_word;
    input_word.data = ((ap_uint<96>)um.i << 64) |
                      ((ap_uint<96>)uy.i << 32) |
                      ((ap_uint<96>)ux.i);
    input_word.last = (i == num_lines - 1) ? 1 : 0;
    input_word.keep = 0xFFF; // For 96-bit TDATA (12 bytes)
    in_stream.write(input_word);
    i++;
  }
}

void display_results(hls::stream<axis_out_t> &out_stream) {
  bool fitness_printed = false;
  int solution_index = 0;
  int count = 0;

  while (!out_stream.empty()) {
    axis_out_t output_word = out_stream.read();
    union {
      float f;
      uint32_t i;
    } val_fitness, val_solution;

    // Extract the full 32-bit value for fitness (no need for bitwise masking)
    uint32_t fitness_bits = output_word.data;  // Fitness is stored in the entire 32 bits

    // Extract the full 32-bit value for solution (if needed)
    uint32_t solution_bits = output_word.data;  // Same logic if fitness and solution are packed together

    // Convert the full 32-bit fitness and solution values into floating point values
    val_fitness.i = fitness_bits;  // Directly use the 32 bits for fitness
    val_solution.i = solution_bits; // Directly use the 32 bits for solution

    if (!fitness_printed) {
      // Print best fitness from the first word
      cout << "Best Fitness: " << fixed << setprecision(6) << val_fitness.f
           << "\n";
      cout << "Best Solution: ";
      fitness_printed = true;
    } else {
      // Print only best solution for subsequent words
      cout << fixed << setprecision(6) << val_solution.f << " ";
    }

    if (output_word.last)
      break;
  }

  // Ensure the stream is fully consumed before exiting
  while (!out_stream.empty()) {
    out_stream.read(); // Drain any leftover words
  }

  cout << "\n";
}

// Main testbench function
int main(int argc, char **argv) {
  if (argc < 2) {
    cerr << "usage: filename.csv>\n" << argv[0];
    return 1;
  }

  hls::stream<axis_in_t> in_stream;
  hls::stream<axis_out_t> out_stream;

  // Load input data into in_stream
  load_data(argv[1], in_stream);

  // Start timing
  auto start_time = chrono::high_resolution_clock::now();

  // Run ACO routine
  cout << "\nRunning ACO minimisation...\n";
  aco(in_stream, out_stream, MAX_ENTRIES);

  // Stop timing
  auto end_time = chrono::high_resolution_clock::now();
  auto elapsed =
      chrono::duration_cast<chrono::duration<double>>(end_time - start_time)
          .count();

  // Print results
  display_results(out_stream);
  cout << "Elapsed Time: " << elapsed << " seconds" << endl;

  while (!in_stream.empty())
    in_stream.read(); // Drain remaining

  return 0;
}
