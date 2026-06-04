#ifndef CROSSOVER_H
#define CROSSOVER_H

/**
 * @file crossover.h
 * @brief Header file for crossover functions used in the genetic algorithm.
 *
 * Provides a hybrid uniform crossover operator that combines:
 * - Gene-by-gene random selection (uniform crossover) for higher genetic diversity.
 * - Deterministic duplicate resolution, guaranteeing no city appears twice in a child individual.
 *
 * The entry point for a full generation is crossover_generation_new(),
 * which internally calls selection_tournament() to choose parents before
 * applying the crossover operator.
 */

#include "generation.h"

/**
 * @brief Produces one child individual from two parents.
 *
 * For each gene position i, the child inherits the gene from parent1 or
 * parent2 with equal probability (coin flip). If the chosen gene is already
 * present in the child (duplicate), the function falls back to a deterministic
 * scan of parent2, then parent1, to find the first unused gene.
 * This guarantees no duplicate hospital city in the child and no infinite loop.
 *
 * @param parent1 First parent individual (read-only).
 * @param parent2 Second parent individual (read-only).
 * @param child   Output individual. Its indices array is allocated inside
 *                this function; the caller must eventually free it via
 *                generation_free() or directly via free(child->indices).
 * @return 0 on success, 1 on invalid input or allocation failure.
 */
int crossover(const Individu *parent1, const Individu *parent2,
              Individu *child);

/**
 * @brief Produces a new generation by crossover with tournament selection.
 *
 * Calls selection_tournament() internally to pick 2 * gen->size parents,
 * then pairs them and applies crossover() to produce gen->size children.
 * If p1 == p2 after selection, a different p2 is drawn at random.
 *
 * @param gen The current generation used as the parent pool.
 * @param tournament_k Tournament size for parent selection.
 * @return A newly allocated Generation containing gen->size children.
 *         On failure, the returned Generation has size == 0 and items == NULL.
 *         The caller is responsible for freeing it via generation_free().
 */
Generation crossover_generation_new(const Generation *gen,
                                    int tournament_k);

#endif /* CROSSOVER_H */