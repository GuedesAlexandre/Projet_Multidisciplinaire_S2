#include "generation.h"
#include <time.h>

static int contains_int(const int* arr, int len, int v) {
    int i;
    for (i = 0; i < len; i++) if (arr[i] == v) return 1;
    return 0;
}

int generation_init_random(Generation* gen,
                           int population_size,
                           int nb_hop,
                           int commune_count) {
    int i, j;
    if (!gen || population_size <= 0 || nb_hop <= 0 || commune_count <= 0) return 1;

    gen->items = (Individu*)malloc(sizeof(Individu) * population_size);
    if (gen->items == NULL) return 1;

    gen->size = population_size;
    gen->capacity = population_size;
    gen->best_index = -1;

    for (i = 0; i < population_size; i++) {
        Individu* ind = &gen->items[i];
        ind->nb_hop = nb_hop;
        ind->fitness = 0.0;
        ind->nb_chru = 0;
        ind->indices = (int*)malloc(sizeof(int) * nb_hop);
        if (!ind->indices) return 1;
  /* ici j'ai besoin de faire en sorte de pas définir deux hopitaux dans les même ville on veut éviter les doublons */
        for (j = 0; j < nb_hop; j++) {
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
