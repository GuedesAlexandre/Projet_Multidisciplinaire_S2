#ifndef GENERATION_H
#define GENERATION_H

#include <stdlib.h>

typedef struct {
    int   *indices;
    int    nb_hop;
    double fitness;
    int    nb_chru;
} Individu;

typedef struct {
    Individu* items;
    int       size;
    int       capacity;
    int       best_index;
} Generation;

int generation_init_random(Generation* gen,
                           int population_size,
                           int nb_hop,
                           int commune_count);

void generation_free(Generation* gen);


#endif /* GENERATION_H */
