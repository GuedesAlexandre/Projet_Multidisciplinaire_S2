#include "selection.h"
#include <stdlib.h>

/**
 * @file selection.c
 * @brief Implementation of selection strategies for the genetic algorithm.
 */

static int already_in(const int *arr, int len, int v)
{
    int i;
    for (i = 0; i < len; i++)
        if (arr[i] == v) return 1;
    return 0;
}

/**
 * For each of the parent_count slots, k individuals are drawn at random
 * from the generation and the one with the highest fitness wins the
 * tournament and is added to out_parent_indices.
 * Duplicates in the output are possible and expected: a strong individual
 * will naturally win multiple tournaments.
 */
int selection_tournament(const Generation *gen,
                         int *out_parent_indices,
                         int parent_count,
                         int k)
{
    int i, j;
    int best, challenger;

    if (!gen || !gen->items || !out_parent_indices) return 1;
    if (parent_count <= 0 || gen->size <= 0 || k <= 0) return 1;
    if (k > gen->size) k = gen->size;

    for (i = 0; i < parent_count; i++) {
        best = rand() % gen->size;
        for (j = 1; j < k; j++) {
            challenger = rand() % gen->size;
            if (gen->items[challenger].fitness > gen->items[best].fitness)
                best = challenger;
        }
        out_parent_indices[i] = best;
    }
    return 0;
}

/**
 * Uses a linear scan repeated elite_count times, skipping indices already
 * selected. Output is ordered by descending fitness.
 */
int selection_find_elites(const Generation *gen,
                          int *out_elite_indices,
                          int elite_count)
{
    int i, e, best;

    if (!gen || !gen->items || !out_elite_indices) return 1;
    if (elite_count <= 0 || elite_count > gen->size) return 1;

    for (e = 0; e < elite_count; e++) {
        best = -1;
        for (i = 0; i < gen->size; i++) {
            if (already_in(out_elite_indices, e, i)) continue;
            if (best == -1 ||
                gen->items[i].fitness > gen->items[best].fitness)
                best = i;
        }
        out_elite_indices[e] = best;
    }
    return 0;
}
