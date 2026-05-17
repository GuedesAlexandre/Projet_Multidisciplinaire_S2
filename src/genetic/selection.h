#ifndef SELECTION_H
#define SELECTION_H

/**
 * @file selection.h
 * @brief Header file for selection functions used in the genetic algorithm.
 *
 * Provides two selection strategies:
 * - Tournament selection: picks parents for crossover by organising
 *   random tournaments among individuals.
 * - Elite selection: identifies the best individuals to carry over
 *   unchanged into the next generation (elitism).
 */

#include "generation.h"

/** @brief Number of individuals competing in each tournament. */
#define TOURNAMENT_K 3

/** @brief Number of elite individuals preserved each generation. */
#define ELITE_COUNT  2

/**
 * @brief Selects parents for crossover using tournament selection.
 * @param gen                The current generation to select from.
 * @param out_parent_indices Output array of size parent_count filled with
 *                           indices into gen->items[].
 * @param parent_count       Number of parents to select.
 * @param k                  Tournament size. Clamped to gen->size if larger.
 * @return 0 on success, 1 if any argument is invalid.
 */
int selection_tournament(const Generation *gen,
                         int *out_parent_indices,
                         int parent_count,
                         int k);

/**
 * @brief Identifies the elite_count best individuals in the generation.
 * @param gen               The current generation to search.
 * @param out_elite_indices Output array of size elite_count filled with
 *                          indices into gen->items[].
 * @param elite_count       Number of elites to identify.
 * @return 0 on success, 1 if any argument is invalid.
 */
int selection_find_elites(const Generation *gen,
                          int *out_elite_indices,
                          int elite_count);

#endif /* SELECTION_H */