#ifndef MUTATION_H
#define MUTATION_H

#include "generation.h"

/*
 * Apply a random mutation on an individual (swap or replace).
 * commune_count: total number of available communes.
 * Returns 0 on success, 1 on error.
 */
int mutation_apply(Individu* ind, int commune_count);

/*
 * Apply mutation_apply to each individual in the generation with probability mutation_rate.
 * start_index allows skipping elites at the beginning of the array.
 * Returns 0 on success, 1 on error.
 */
int mutation_generation(Generation* gen,
                        int commune_count,
                        double mutation_rate,
                        int start_index);

#endif /* MUTATION_H */
