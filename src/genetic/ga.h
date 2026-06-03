#ifndef GA_H
#define GA_H

#include "generation.h"
#include "coverage.h"
#include "../data/municipality.h"

/**
 * @brief Configuration parameters for the genetic algorithm.
 */
typedef struct {
    int population_size;
    int nb_hop;
    int max_generations;
    int elite_count;
    int tournament_k;
    double mutation_rate;
    double mutation_rate_start;
    double mutation_rate_end;
    double immigrants_rate;
    double immigrants_rate_max;
    double selection_random_rate;
    double guided_mutation_rate;
    int restart_threshold;
    int max_restarts;
    double target_fitness;
    int stagnation_limit;
    int verbose;
} GAConfig;

/**
 * @brief Returns a default configuration tuned for the project.
 */
GAConfig ga_config_default(void);

/**
 * @brief Initializes a random population based on the config.
 * @return 0 on success, 1 on error.
 */
int ga_init_generation(Generation *gen,
                        const GAConfig *config,
                        const MunicipalityTable *table);

/**
 * @brief Computes fitness for all individuals in the generation.
 */
void ga_evaluate_generation(const MunicipalityTable *table,
                            const CoverageIndex *coverage,
                            Generation *gen);

/**
 * @brief Produces the next generation (elitism + tournament + crossover).
 * @return A new generation (size > 0) or an empty generation on failure.
 */
Generation ga_next_generation(const Generation *current,
                              const GAConfig *config,
                              const MunicipalityTable *table,
                              const CoverageIndex *coverage);

/**
 * @brief Returns the best fitness in a generation.
 */
double ga_get_best_fitness(const Generation *gen);

/**
 * @brief Returns the average fitness in a generation.
 */
double ga_get_avg_fitness(const Generation *gen);

/**
 * @brief Returns detailed stats for the best individual.
 */
void ga_get_best_stats(const MunicipalityTable *table,
                       const CoverageIndex *coverage,
                       const Generation *gen,
                       double *out_fitness,
                       long *out_habitants_desert,
                       int *out_nb_chru);

/**
 * @brief Reseeds a generation while keeping a number of elites.
 */
int ga_reseed_generation(Generation *gen,
                         const GAConfig *config,
                         const MunicipalityTable *table,
                         int keep_elites);

/**
 * @brief Applies a guided mutation that targets uncovered communes.
 */
int ga_guided_mutation(Generation *gen,
                       const MunicipalityTable *table,
                       const CoverageIndex *coverage,
                       double guided_rate,
                       int start_index);

#endif /* GA_H */
