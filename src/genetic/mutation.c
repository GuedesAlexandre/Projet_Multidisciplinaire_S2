#include "mutation.h"
#include <stdlib.h>

static int contains_int(const int* arr, int len, int v) {
    int i;
    for (i = 0; i < len; i++) {
        if (arr[i] == v) return 1;
    }
    return 0;
}

/*
 * Replace a randomly picked hospital with another commune,
 * ensuring no duplicates.
 */
static void mutation_deplacement(Individu* ind, int commune_count) {
    int pos; 
    int cand;

    pos  = rand() % ind->nb_hop;
    cand = rand() % commune_count;
    while (contains_int(ind->indices, ind->nb_hop, cand)) {
        cand = rand() % commune_count;
    }
    ind->indices[pos] = cand;
}

/*
 * Add a commune to indices[].
 * Returns 0 on success, 1 if realloc fails.
 */
static int mutation_ajout(Individu* ind, int commune_count) {
    int cand;
    int* tmp;

    tmp = (int*)realloc(ind->indices, sizeof(int) * (ind->nb_hop + 1));
    if (!tmp) return 1;
    ind->indices = tmp;

    cand = rand() % commune_count;
    while (contains_int(ind->indices, ind->nb_hop, cand)) {
        cand = rand() % commune_count;
    }
    ind->indices[ind->nb_hop] = cand;
    ind->nb_hop++;
    return 0;
}

/*
 * Remove a random hospital using a swap remove (O(1)).
 * Does nothing if nb_hop <= 1.
 */
static void mutation_suppression(Individu* ind) {
    int pos;
    if (ind->nb_hop <= 1) return;
    pos = rand() % ind->nb_hop;
    ind->indices[pos] = ind->indices[ind->nb_hop - 1];
    ind->nb_hop--;
}

int mutation_apply(Individu* ind, int commune_count) {
    int op;
    if (!ind || ind->nb_hop <= 0 || commune_count <= 0) return 1;
    op = rand() % 3;
    if (op == 0) {
        mutation_deplacement(ind, commune_count);
        return 0;
    } else if (op == 1) {
        return mutation_ajout(ind, commune_count);
    } else {
        mutation_suppression(ind);
        return 0;
    }
}

int mutation_generation(Generation* gen, int commune_count) {
    int i;
    double r;
    if (!gen || commune_count <= 0) return 1;
    for (i = 0; i < gen->size; i++) {
        r = (double)rand() / RAND_MAX;
        if (r < MUTATION_RATE) {
            if (mutation_apply(&gen->items[i], commune_count) != 0)
                return 1;
        }
    }
    return 0;
}
