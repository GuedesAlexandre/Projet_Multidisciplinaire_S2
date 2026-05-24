#include "selection.h"
#include <stdlib.h>
#include "../tools/tools.h"

typedef struct {
    int index;
    double fitness;
} FitnessIndex;

static int compare_fitness_desc(const void *a, const void *b)
{
    const FitnessIndex *fa = (const FitnessIndex *)a;
    const FitnessIndex *fb = (const FitnessIndex *)b;
    if (fa->fitness < fb->fitness) return 1;
    if (fa->fitness > fb->fitness) return -1;
    return 0;
}

/**
 * @file selection.c
 * @brief Implementation of selection strategies for the genetic algorithm.
 */

/**
 * @brief Selects one parent index using tournament selection.
 */
int selection_tournament_pick(const Generation *gen, int k)
{
    int j;
    int best, challenger;

    if (!gen || !gen->items) return -1;
    if (gen->size <= 0 || k <= 0) return -1;
    if (k > gen->size) k = gen->size;

    best = rand() % gen->size;
    for (j = 1; j < k; j++) {
        challenger = rand() % gen->size;
        if (gen->items[challenger].fitness > gen->items[best].fitness)
            best = challenger;
    }
    return best;
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
    int i;
    int pick;

    if (!gen || !gen->items || !out_parent_indices) return 1;
    if (parent_count <= 0 || gen->size <= 0 || k <= 0) return 1;

    for (i = 0; i < parent_count; i++) {
        pick = selection_tournament_pick(gen, k);
        if (pick < 0) return 1;
        out_parent_indices[i] = pick;
    }
    return 0;
}

int selection_rank_build(const Generation *gen,
                         int **out_indices,
                         double **out_cdf,
                         int *out_count)
{
    int i;
    int count;
    FitnessIndex *arr;
    int *indices;
    double *cdf;
    double total;
    double running;

    if (!gen || !gen->items || !out_indices || !out_cdf || !out_count)
        return 1;
    if (gen->size <= 0) return 1;

    count = gen->size;

    arr = (FitnessIndex *)malloc(sizeof(FitnessIndex) * count);
    if (!arr) return 1;

    for (i = 0; i < count; i++) {
        arr[i].index = i;
        arr[i].fitness = gen->items[i].fitness;
    }

    qsort(arr, count, sizeof(FitnessIndex), compare_fitness_desc);

    indices = (int *)malloc(sizeof(int) * count);
    cdf = (double *)malloc(sizeof(double) * count);
    if (!indices || !cdf) {
        if (indices) free(indices);
        if (cdf) free(cdf);
        free(arr);
        return 1;
    }

    total = (count * (count + 1)) / 2.0;
    running = 0.0;

    for (i = 0; i < count; i++) {
        double weight = (count - i);
        indices[i] = arr[i].index;
        running += weight;
        cdf[i] = running / total;
    }

    free(arr);

    *out_indices = indices;
    *out_cdf = cdf;
    *out_count = count;
    return 0;
}

int selection_rank_pick(const int *indices, const double *cdf, int count)
{
    double r;
    int lo, hi, mid;

    if (!indices || !cdf || count <= 0) return -1;

    r = rand() / RAND_MAX;

    lo = 0;
    hi = count - 1;
    while (lo < hi) {
        mid = lo + (hi - lo) / 2;
        if (r <= cdf[mid]) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }

    return indices[lo];
}

void selection_rank_free(int *indices, double *cdf)
{
    if (indices) free(indices);
    if (cdf) free(cdf);
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
            if (contains_int(out_elite_indices, e, i)) continue;
            if (best == -1 ||
                gen->items[i].fitness > gen->items[best].fitness)
                best = i;
        }
        out_elite_indices[e] = best;
    }
    return 0;
}
