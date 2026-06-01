#include "mutation.h"
#include <stdlib.h>
#include "../tools/tools.h"

static void mutation_replace(Individu* ind, int commune_count) {
    int pos;
    int cand;

    if (ind->nb_hop <= 0) return;
    if (ind->nb_hop >= commune_count) return;
    pos = rand() % ind->nb_hop;
    cand = rand() % commune_count;
    int tries = 0;
    while (contains_int(ind->indices, ind->nb_hop, cand) && tries < 100) {
        cand = rand() % commune_count;
        tries++;
    }
    ind->indices[pos] = cand;
}

static void mutation_remove(Individu* ind) {
    if (ind->nb_hop <= 1) return; /* Keep at least one */
    int pos = rand() % ind->nb_hop;
    ind->indices[pos] = ind->indices[ind->nb_hop - 1];
    ind->nb_hop--;
    int *new_indices = (int*)realloc(ind->indices, sizeof(int) * ind->nb_hop);
    if (new_indices) ind->indices = new_indices;
}


static void mutation_add(Individu* ind, int commune_count) {
    if (ind->nb_hop >= commune_count) return;
    int cand = rand() % commune_count;
    int tries = 0;
    while (contains_int(ind->indices, ind->nb_hop, cand) && tries < 100) {
        cand = rand() % commune_count;
        tries++;
    }
    ind->nb_hop++;
    int *new_indices = (int*)realloc(ind->indices, sizeof(int) * ind->nb_hop);
    if (new_indices) {
        ind->indices = new_indices;
        ind->indices[ind->nb_hop - 1] = cand;
    } else {
        ind->nb_hop--;
    }
}

int mutation_apply(Individu* ind, int commune_count) {
    int op;
    if (!ind || ind->nb_hop <= 0 || commune_count <= 0) return 1;
    op = rand() % 100;
    
    if(op < 45) {
        mutation_replace(ind, commune_count);
    } else if (op < 80) {
        mutation_remove(ind);
    } else {
        mutation_add(ind, commune_count);
    }
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
