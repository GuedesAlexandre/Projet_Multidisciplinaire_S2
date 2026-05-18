#ifndef MUTATION_H
#define MUTATION_H

#include "generation.h"

#define MUTATION_RATE 0.05

/*
 * Apply a random mutation on an individual (move, add, or remove).
 * commune_count: total number of available communes.
 * Returns 0 on success, 1 on memory error.
 */
int mutation_apply(Individu* ind, int commune_count);

/*
 * Apply mutation_apply to each individual in the generation with probability MUTATION_RATE.
 * Returns 0 on success, 1 on memory error.
 */
int mutation_generation(Generation* gen, int commune_count);

#endif /* MUTATION_H */
