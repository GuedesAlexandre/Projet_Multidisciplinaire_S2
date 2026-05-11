#include "crossover.h"
#include "selection.h"
#include <stdlib.h>

/**
 * @file crossover.c
 * @brief Implementation of the hybrid crossover operator.
 */

/**
 * @brief Checks whether a value is already present in an integer array.
 * @param arr Array to search (indices already placed in the child).
 * @param len Number of elements already placed (indices 0..len-1).
 * @param v   Candidate gene value to check.
 * @return 1 if v is already in arr[0..len-1], 0 otherwise.
 */
static int contains(const int *arr, int len, int v)
{
    int i;
    for (i = 0; i < len; i++)
        if (arr[i] == v) return 1;
    return 0;
}

/**
 * @brief Produces one child individual from two parents using hybrid crossover.
 *
 * Step 1 — Coin flip: for position i, pick parent1->indices[i] or
 *           parent2->indices[i] with equal probability.
 * Step 2 — No duplicate: keep it.
 * Step 3 — Duplicate found: scan parent2 in order for the first unused gene.
 * Step 4 — Still duplicate: scan parent1 in order for the first unused gene.
 * Step 5 — A valid candidate is always found because both parents contain
 *           nb_hop distinct indices.
 *
 * @param parent1 First parent (read-only).
 * @param parent2 Second parent (read-only).
 * @param child   Output child; indices[] is allocated here.
 * @param nb_hop  Number of hospitals per individual.
 * @return 0 on success, 1 on error.
 */
int crossover(const Individu *parent1, const Individu *parent2,
              Individu *child, int nb_hop)
{
    int i, j, found;
    int candidate;

    if (!parent1 || !parent2 || !child) return 1;
    if (!parent1->indices || !parent2->indices) return 1;

    child->indices = (int *)malloc(sizeof(int) * nb_hop);
    if (!child->indices) return 1;
    child->nb_hop  = nb_hop;
    child->fitness = 0.0;
    child->nb_chru = 0;

    for (i = 0; i < nb_hop; i++) {

        /* Step 1: coin flip */
        candidate = ((rand() % 2) == 0)
                    ? parent1->indices[i]
                    : parent2->indices[i];

        /* Step 2: no duplicate — keep it */
        if (!contains(child->indices, i, candidate)) {
            child->indices[i] = candidate;
            continue;
        }

        /* Step 3: duplicate — deterministic scan of parent2 */
        found = 0;
        for (j = 0; j < nb_hop && !found; j++) {
            if (!contains(child->indices, i, parent2->indices[j])) {
                candidate = parent2->indices[j];
                found = 1;
            }
        }

        /* Step 4: still duplicate — deterministic scan of parent1 */
        if (!found) {
            for (j = 0; j < nb_hop && !found; j++) {
                if (!contains(child->indices, i, parent1->indices[j])) {
                    candidate = parent1->indices[j];
                    found = 1;
                }
            }
        }

        /* Step 5: found is always 1 here */
        child->indices[i] = candidate;
    }

    return 0;
}

/**
 * @brief Produces a new generation via tournament selection and crossover.
 *
 * Selects 2 * gen->size parents using tournament selection, then pairs
 * them to produce 2 * gen->size children. Each pair (p1, p2) yields two
 * children: crossover(p1, p2) and crossover(p2, p1).
 *
 * @param gen    Current generation (parent pool).
 * @param nb_hop Number of hospitals per individual.
 * @return Newly allocated Generation with 2 * gen->size individuals,
 *         or an empty Generation (size == 0) on failure.
 */
Generation crossover_generation_new(const Generation *gen, int nb_hop)
{
    Generation children;
    int *parents;
    int i, p1, p2;

    children.items      = NULL;
    children.size       = 0;
    children.capacity   = 0;
    children.best_index = -1;

    if (!gen || nb_hop <= 0) return children;

    parents = (int *)malloc(sizeof(int) * gen->size * 2);
    if (!parents) return children;

    if (selection_tournament(gen, parents, gen->size * 2, TOURNAMENT_K) != 0) {
        free(parents);
        return children;
    }

    children.items = (Individu *)malloc(sizeof(Individu) * gen->size * 2);
    if (!children.items) { free(parents); return children; }
    children.size     = gen->size * 2;
    children.capacity = gen->size * 2;

    for (i = 0; i < gen->size * 2; i += 2) {
        p1 = parents[i];
        p2 = parents[i + 1];

        /* Ensure the two parents are distinct */
        while (p2 == p1) p2 = rand() % gen->size;

        if (crossover(&gen->items[p1], &gen->items[p2],
                      &children.items[i], nb_hop) != 0) {
            children.size = i;
            generation_free(&children);
            children.size = 0;
            free(parents);
            return children;
        }
        if (crossover(&gen->items[p2], &gen->items[p1],
                      &children.items[i + 1], nb_hop) != 0) {
            children.size = i + 1;
            generation_free(&children);
            children.size = 0;
            free(parents);
            return children;
        }
    }

    free(parents);
    return children;
}
