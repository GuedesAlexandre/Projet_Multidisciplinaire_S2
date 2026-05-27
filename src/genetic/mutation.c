#include "mutation.h"
#include <stdlib.h>
#include "../tools/tools.h"
/*
 * Swap two positions inside the same individual.
 */
static void mutation_swap(Individu* ind) {
    int a;
    int b;
    int tmp;
    if (ind->nb_hop < 2) return;
    a = rand() % ind->nb_hop;
    b = rand() % ind->nb_hop;
    if (a == b) return;
    tmp = ind->indices[a];
    ind->indices[a] = ind->indices[b];
    ind->indices[b] = tmp;
}

/*
 * Replace a hospital with another commune, ensuring no duplicates.
 */
static void mutation_replace(Individu* ind, int commune_count) {
    int pos;
    int cand;

    if (ind->nb_hop >= commune_count) return;
    pos = rand() % ind->nb_hop;
    cand = rand() % commune_count;
    while (contains_int(ind->indices, ind->nb_hop, cand)) {
        cand = rand() % commune_count;
    }
    ind->indices[pos] = cand;
}

int mutation_apply(Individu* ind, int commune_count) {
    int op;
    if (!ind || ind->nb_hop <= 0 || commune_count <= 0) return 1;
    op = rand() % 2;
    if (op == 0) {
        mutation_replace(ind, commune_count);
        return 0;
    }
    mutation_swap(ind);
    return 0;
}

int mutation_generation(Generation* gen,
                        int commune_count,
                        double mutation_rate,
                        int start_index) {
    int i;
    double r;
    if (!gen || commune_count <= 0) return 1;
    if (mutation_rate <= 0.0) return 0;
    if (start_index < 0) start_index = 0;
    if (start_index >= gen->size) return 0;

    for (i = start_index; i < gen->size; i++) {
        r = (double)rand() / RAND_MAX;
        if (r < mutation_rate) {
            if (mutation_apply(&gen->items[i], commune_count) != 0)
                return 1;
        }
    }
    return 0;
}
