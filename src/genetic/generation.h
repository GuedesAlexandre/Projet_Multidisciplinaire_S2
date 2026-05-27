#ifndef GENERATION_H
#define GENERATION_H

#include <stdlib.h>
#include "../data/municipality.h"

/*
@struct Individu
@brief Represents an individual solution in the genetic algorithm, encoding a set of hospital locations.
- indices: An array of integers, each representing the index of a municipality chosen as a hospital location.
- nb_hop: The number of hospitals (length of the indices array).
- fitness: The computed fitness score of the individual, where higher is better.
- nb_chru: The number of CHRU (Centres Hospitaliers Régionaux Universitaires for cityes with population >= CHRU_POP_THRESHOLD) covered by the individual's hospital locations.

*/
typedef struct {
    int   *indices;
    int    nb_hop;
    double fitness;
    int    nb_chru;
} Individu;

/*
@struct Generation
@brief Represents a population of individuals in the genetic algorithm.
- items: An array of Individu, representing the individuals in the generation.
- size: The current number of individuals in the generation.
- capacity: The allocated capacity of the items array (should be >= size).
- best_index: The index of the individual with the highest fitness in the items array, or -1 if the generation is empty or not evaluated.
*/
typedef struct {
    Individu* items;
    int       size;
    int       capacity;
    int       best_index;
} Generation;

/*
@brief Initializes a generation with random individuals based on uniform selection.
- gen: Pointer to the Generation to initialize. Its items array will be allocated and filled with random individuals.
- population_size: The number of individuals to generate.
- nb_hop: The number of hospitals (genes) each individual should have.
- commune_count: The total number of municipalities available (used to ensure valid indices).
@return 0 on success, 1 on error (e.g., invalid parameters or memory allocation failure).
*/
int generation_init_random(Generation* gen,
                           int population_size,
                           int nb_hop,
                           int commune_count);

/*
@brief Initializes a generation with random individuals based on weighted selection, where municipalities with larger populations are more likely to be chosen as hospital locations.
- gen: Pointer to the Generation to initialize. Its items array will be allocated and filled with random individuals.
- population_size: The number of individuals to generate.
- nb_hop: The number of hospitals (genes) each individual should have.
- table: Pointer to the MunicipalityTable containing the municipalities and their populations, used to compute selection weights.
@return 0 on success, 1 on error (e.g., invalid parameters or memory allocation failure).
*/                      
int generation_init_weighted(Generation* gen,
                             int population_size,
                             int nb_hop,
                             const MunicipalityTable* table);

void generation_free(Generation* gen);


#endif /* GENERATION_H */
