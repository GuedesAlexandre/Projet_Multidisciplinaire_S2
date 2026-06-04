#include "generation.h"
#include <time.h>
#include "../tools/tools.h"


int generation_init_random(Generation* gen,
                           int population_size,
                           int nb_hop,
                           int commune_count) {
    int i, j;
    if (!gen || population_size <= 0 || nb_hop <= 0 || commune_count <= 0) return 1;
    if (nb_hop > commune_count) return 1;

    gen->items = (Individu*)malloc(sizeof(Individu) * population_size);
    if (gen->items == NULL) return 1;

    gen->size = population_size;
    gen->capacity = population_size;
    gen->best_index = -1;

    for (i = 0; i < population_size; i++) {
        int initial_hops = 1 + rand() % nb_hop;
        Individu* ind = &gen->items[i];
        ind->nb_hop = initial_hops;
        ind->fitness = 0.0;
        ind->nb_chru = 0;
        ind->indices = NULL;
        ind->indices = (int*)malloc(sizeof(int) * initial_hops);
        if (!ind->indices) {
            gen->size = i;
            generation_free(gen);
            return 1;
        }
        for (j = 0; j < initial_hops; j++) {
            int cand = rand() % commune_count;
            while (contains_int(ind->indices, j, cand)) cand = rand() % commune_count;
            ind->indices[j] = cand;
        }
    }
    return 0;
}

void generation_free(Generation* gen) {
    int i;
    if (!gen) return;
    if (gen->items) {
        for (i = 0; i < gen->size; i++) {
            free(gen->items[i].indices);
            gen->items[i].indices = NULL;
        }
        free(gen->items);
        gen->items = NULL;
    }
    gen->size = 0;
    gen->capacity = 0;
    gen->best_index = -1;
}
