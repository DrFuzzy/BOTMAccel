#ifndef ACO_MINIMIZATION_H
#define ACO_MINIMIZATION_H

#define DIMENSIONS 6 // Number of parameters

void aco(const float ownship_x[], const float ownship_y[],
         const float measure[], const float timeframe[], float &best_fitness,
         float best_solution[DIMENSIONS], int n);

#endif // ACO_MINIMIZATION_H
