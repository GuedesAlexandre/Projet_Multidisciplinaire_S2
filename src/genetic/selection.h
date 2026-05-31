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



/**
 * @brief Selects one parent index using tournament selection.
 * @param gen The current generation to select from.
 * @param k   Tournament size. Clamped to gen->size if larger.
 * @return The selected index, or -1 on error.
 */
int selection_tournament_pick(const Generation *gen, int k);

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
 * @brief Prepares a rank-based selection CDF.
 * @return 0 on success, 1 on error.
 */
int selection_rank_build(const Generation *gen,
                         int **out_indices,
                         double **out_cdf,
                         int *out_count);

/**
 * @brief Picks one index using the rank CDF.
 * @return The selected index, or -1 on error.
 */
int selection_rank_pick(const int *indices, const double *cdf, int count);

/**
 * @brief Frees rank selection buffers.
 */
void selection_rank_free(int *indices, double *cdf);

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