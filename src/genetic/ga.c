#include "ga.h"
#include "fitness.h"
#include "selection.h"
#include "crossover.h"
#include "mutation.h"
#include <stdlib.h>
#include <limits.h>
#include "../tools/tools.h"


GAConfig ga_config_default(void)
{
    GAConfig cfg;
    cfg.population_size = 800;
    cfg.nb_hop = 2000;
    cfg.max_generations = 1000;
    cfg.elite_count = 2;
    cfg.tournament_k = 5;
    cfg.mutation_rate_start = 0.30;
    cfg.mutation_rate_end = 0.05;
    cfg.mutation_rate = cfg.mutation_rate_start;
    cfg.immigrants_rate = 0.10;
    cfg.immigrants_rate_max = 0.30;
    cfg.selection_random_rate = 0.10;
    cfg.guided_mutation_rate = 0.40;
    cfg.restart_threshold = 30;
    cfg.max_restarts = 5;
    cfg.target_fitness = 52000000.0;
    cfg.stagnation_limit = 120;
    cfg.verbose = 1;
    return cfg;
}

int ga_init_generation(Generation *gen,
                        const GAConfig *config,
                        const MunicipalityTable *table)
{
    if (!config || !table) return 1;
    return generation_init_random(gen, config->population_size, config->nb_hop, (int)table->count);
}

static double compute_fitness_indexed(const MunicipalityTable *table,
                                      const CoverageIndex *coverage,
                                      const int *hospital_indices,
                                      int hospital_count,
                                      long *out_habitants_desert,
                                      long *out_communes_desert,
                                      int *out_nb_chru,
                                      int *stamp,
                                      int stamp_id)
{
    size_t i;
    int j;
    long habitants_desert = 0;
    long communes_desert = 0;
    int nb_chru = 0;

    if (!table || !coverage || !hospital_indices || hospital_count <= 0)
        return 0;

    for (j = 0; j < hospital_count; j++) {
        int hi = hospital_indices[j];
        if (hi >= 0 && hi < coverage->count) {
            if (table->items[hi].population >= CHRU_POP_THRESHOLD)
                nb_chru++;

            for (i = (size_t)coverage->offsets[hi];
                 i < (size_t)coverage->offsets[hi + 1];
                 i++) {
                int covered_idx = coverage->indices[i];
                stamp[covered_idx] = stamp_id;
            }
        }
    }

    for (i = 0; i < table->count; i++) {
        if (stamp[i] != stamp_id) {
            habitants_desert += table->items[i].population;
            communes_desert++;
        }
    }

    if (out_habitants_desert) *out_habitants_desert = habitants_desert;
    if (out_communes_desert)  *out_communes_desert  = communes_desert;
    if (out_nb_chru) *out_nb_chru = nb_chru;

    return (double) FITNESS_BASE_POPULATION
         - habitants_desert
         - (double)(FITNESS_PENALTY_HOP * hospital_count)
         + (double)(FITNESS_BONUS_CHRU * nb_chru); /*Je suis ici obligé de caster en double pour éviter un overflow dans le calcul du bonus CHRU, qui peut dépasser la capacité d'un int sur 32 bits*/
}

void ga_evaluate_generation(const MunicipalityTable *table,
                            const CoverageIndex *coverage,
                            Generation *gen)
{
    int i;
    long habitants_desert;
    int nb_chru;
    int stamp_id = 1;
    int *stamp;

    if (!table || !coverage || !gen || !gen->items) return;
    if ((int)table->count != coverage->count) return;

    stamp = (int *)calloc(table->count, sizeof(int));
    if (!stamp) return;

    for (i = 0; i < gen->size; i++) {
        {
            long communes_desert = 0;
            gen->items[i].fitness = compute_fitness_indexed(table,
                                                            coverage,
                                                            gen->items[i].indices,
                                                            gen->items[i].nb_hop,
                                                            &habitants_desert,
                                                            &communes_desert,
                                                            &nb_chru,
                                                            stamp,
                                                            stamp_id);
            gen->items[i].nb_chru          = nb_chru;
            gen->items[i].habitants_desert = habitants_desert;
            gen->items[i].communes_desert  = communes_desert;
        }

        stamp_id++;
        if (stamp_id == INT_MAX) {
            int k;
            for (k = 0; k < (int)table->count; k++) stamp[k] = 0;
            stamp_id = 1;
        }
    }

    free(stamp);
}

static int randomize_individual_uniform(Individu *ind,
                                        int max_nb_hop,
                                        int commune_count)
{
    int j;
    int initial_hops = 1 + rand() % max_nb_hop;
    if (!ind || max_nb_hop <= 0 || commune_count <= 0) return 1;
    if (max_nb_hop > commune_count) return 1;

    if (ind->indices) {
        free(ind->indices);
        ind->indices = NULL;
    }

    ind->indices = (int *)malloc(sizeof(int) * initial_hops);
    if (!ind->indices) return 1;

    for (j = 0; j < initial_hops; j++) {
        int cand = rand() % commune_count;
        while (contains_int(ind->indices, j, cand))
            cand = rand() % commune_count;
        ind->indices[j] = cand;
    }

    ind->nb_hop = initial_hops;
    ind->fitness = 0.0;
    ind->nb_chru = 0;
    return 0;
}

static int copy_individual(const Individu *src, Individu *dst)
{
    int j;
    if (!src || !dst || !src->indices || src->nb_hop <= 0) return 1;

    if (dst->indices) {
        free(dst->indices);
        dst->indices = NULL;
    }

    dst->indices = (int *)malloc(sizeof(int) * src->nb_hop);
    if (!dst->indices) return 1;

    for (j = 0; j < src->nb_hop; j++)
        dst->indices[j] = src->indices[j];

    dst->nb_hop = src->nb_hop;
    dst->fitness = src->fitness;
    dst->nb_chru = src->nb_chru;
    return 0;
}

static int selection_pick_tournament_mixed(const Generation *gen,
                                           int tournament_k,
                                           double random_rate)
{
    double r;
    if (!gen || gen->size <= 0) return -1;
    if (random_rate < 0.0) random_rate = 0.0;
    if (random_rate > 1.0) random_rate = 1.0;

    r = (double)rand() / (double)RAND_MAX; /* ici je suis obligé de caster en double pour éviter un overflow dans le calcul du modulo */
    if (r < random_rate) return rand() % gen->size; /* ici on récupère un individu au hasard */

    return selection_tournament_pick(gen, tournament_k); /* sinon on utilise la sélection par tournoi classique */
}

static int ga_init_empty_generation(Generation *next, int pop_size, int nb_hop)
{
    int i;
    next->items = (Individu *)malloc(sizeof(Individu) * pop_size);
    if (!next->items) return 1;

    next->size = pop_size;
    next->capacity = pop_size;
    next->best_index = -1;

    for (i = 0; i < pop_size; i++) {
        next->items[i].indices = NULL;
        next->items[i].nb_hop = nb_hop;
        next->items[i].fitness = 0.0;
        next->items[i].nb_chru = 0;
    }
    return 0;
}

static int ga_handle_elitism(const Generation *current, Generation *next, int elite_count)
{
    int i;
    int *elite_indices;

    if (elite_count <= 0) return 0;

    elite_indices = (int *)malloc(sizeof(int) * elite_count);
    if (!elite_indices) return 1;

    if (selection_find_elites(current, elite_indices, elite_count) != 0) {
        free(elite_indices);
        return 1;
    }

    for (i = 0; i < elite_count; i++) {
        if (copy_individual(&current->items[elite_indices[i]], &next->items[i]) != 0) {
            free(elite_indices);
            return 1;
        }
    }

    free(elite_indices);
    return 0;
}

static void ga_pick_distinct_parents(const Generation *gen, int tournament_k, double random_rate, int *p1, int *p2)
{
    int tries = 0;
    *p1 = selection_pick_tournament_mixed(gen, tournament_k, random_rate);
    *p2 = selection_pick_tournament_mixed(gen, tournament_k, random_rate);

    if (*p1 >= 0 && gen->size > 1) {
        while (*p2 == *p1 && tries < 10) {
            *p2 = selection_pick_tournament_mixed(gen, tournament_k, random_rate);
            tries++;
        }
    }
    if (*p1 == *p2 && gen->size > 1) {
        *p2 = (*p1 + 1) % gen->size;
    }
}

static int ga_handle_crossover(const Generation *current, Generation *next, const GAConfig *config, int elite_count)
{
    int i, p1, p2;
    int pop_size = next->size;
    int tournament_k = config->tournament_k;
    double random_rate = config->selection_random_rate;

    if (tournament_k <= 0) tournament_k = 1;
    if (tournament_k > current->size) tournament_k = current->size;

    if (random_rate < 0.0) random_rate = 0.0;
    if (random_rate > 1.0) random_rate = 1.0;

    for (i = elite_count; i < pop_size; i++) {
        ga_pick_distinct_parents(current, tournament_k, random_rate, &p1, &p2);
        if (p1 < 0 || p2 < 0) return 1;

        if (crossover(&current->items[p1], &current->items[p2],
                      &next->items[i]) != 0) {
            return 1;
        }
    }
    return 0;
}

static int ga_handle_immigrants(Generation *next, double immigrants_rate, int commune_count, int elite_count, int nb_hop)
{
    int i, immigrants, immigrants_start;
    int pop_size = next->size;

    if (immigrants_rate < 0.0) immigrants_rate = 0.0;
    if (immigrants_rate > 1.0) immigrants_rate = 1.0;

    immigrants = (int)(immigrants_rate * (double)pop_size);
    if (immigrants > (pop_size - elite_count)) {
        immigrants = pop_size - elite_count;
    }

    if (immigrants > 0) {
        immigrants_start = pop_size - immigrants;
        for (i = immigrants_start; i < pop_size; i++) {
            if (randomize_individual_uniform(&next->items[i], nb_hop, commune_count) != 0) {
                return 1;
            }
        }
    }
    return 0;
}

Generation ga_next_generation(const Generation *current,
                              const GAConfig *config,
                              const MunicipalityTable *table,
                              const CoverageIndex *coverage)
{
    Generation next;
    int pop_size;
    int elite_count;
    int commune_count;

    next.items = NULL;
    next.size = 0;
    next.capacity = 0;
    next.best_index = -1;

    if (!current || !config || !current->items || !table || !coverage) return next;
    if (current->size <= 0) return next;

    commune_count = (int)table->count;

    if (config->nb_hop <= 0 || commune_count <= 0) return next;
    if (config->nb_hop > commune_count) return next;

    pop_size = current->size;
    if (ga_init_empty_generation(&next, pop_size, config->nb_hop) != 0) {
        return next;
    }

    elite_count = config->elite_count;
    if (elite_count < 0) elite_count = 0;
    if (elite_count > pop_size) elite_count = pop_size;

    if (ga_handle_elitism(current, &next, elite_count) != 0) {
        generation_free(&next);
        next.size = 0;
        return next;
    }

    if (ga_handle_crossover(current, &next, config, elite_count) != 0) {
        generation_free(&next);
        next.size = 0;
        return next;
    }

    /* 1. Mutation classique */
    if (mutation_generation(&next, commune_count, config->mutation_rate, elite_count) != 0) {
        generation_free(&next);
        next.size = 0;
        return next;
    }

    /* 2. Mutation Guidée (Ciblée sur les déserts médicaux) */
    if (ga_guided_mutation(&next, table, coverage, config->guided_mutation_rate, 0) != 0) {
        generation_free(&next);
        next.size = 0;
        return next;
    }

    if (ga_handle_immigrants(&next, config->immigrants_rate, commune_count, elite_count, config->nb_hop) != 0) {
        generation_free(&next);
        next.size = 0;
        return next;
    }

    return next;
}

double ga_get_best_fitness(const Generation *gen)
{
    int i;
    double best;
    if (!gen || gen->size <= 0) return 0.0;

    best = gen->items[0].fitness;
    for (i = 1; i < gen->size; i++)
        if (gen->items[i].fitness > best) best = gen->items[i].fitness;
    return best;
}

double ga_get_avg_fitness(const Generation *gen)
{
    int i;
    double avg;
    if (!gen || gen->size <= 0) return 0.0;

    avg = 0.0;
    for (i = 0; i < gen->size; i++) avg += gen->items[i].fitness;
    return avg / gen->size;
}

void ga_get_best_stats(const MunicipalityTable *table,
                       const CoverageIndex *coverage,
                       const Generation *gen,
                       double *out_fitness,
                       long *out_habitants_desert,
                       int *out_nb_chru)
{
    int i;
    int best_index;
    long habitants_desert = 0;
    int nb_chru = 0;
    double fitness = 0.0;

    if (!table || !coverage || !gen || !gen->items || gen->size <= 0) return;
    if ((int)table->count != coverage->count) return;

    best_index = 0;
    for (i = 1; i < gen->size; i++) {
        if (gen->items[i].fitness > gen->items[best_index].fitness)
            best_index = i;
    }

    if (gen->items[best_index].indices) {
        int *stamp = (int *)calloc(table->count, sizeof(int));
        if (!stamp) return;

        fitness = compute_fitness_indexed(table,
                                          coverage,
                                          gen->items[best_index].indices,
                                          gen->items[best_index].nb_hop,
                                          &habitants_desert,
                                          NULL,
                                          &nb_chru,
                                          stamp,
                                          1);
        free(stamp);
    }

    if (out_fitness) *out_fitness = fitness;
    if (out_habitants_desert) *out_habitants_desert = habitants_desert;
    if (out_nb_chru) *out_nb_chru = nb_chru;
}
/* Nombre d'ajouts rentables tentes par individu et taille de l'echantillon. */
  #define GUIDED_MAX_ROUNDS    5 /* Cette constante contrôle le nombre maximal de rounds de mutation guidée. */
  #define GUIDED_SAMPLE_TRIES  40 /* Cette constante contrôle le nombre maximal de tentatives d'échantillonnage pour chaque round. */
  /* Des rounds ? Des échantillons ?
   * Un round est une itération de mutation guidée, un échantillon est un sous-ensemble de la population.
    */

  int ga_guided_mutation(Generation *gen,
                         const MunicipalityTable *table,
                         const CoverageIndex *coverage,
                         double guided_rate,
                         int start_index)
  {
      int i, j, k;
      int count;
      int *cover_count;

      if (!gen || !table || !coverage || !gen->items) return 1;
      if ((int)table->count != coverage->count) return 1;
      if (guided_rate <= 0.0) return 0;
      if (start_index < 0) start_index = 0;
      if (start_index >= gen->size) return 0;

      count = (int)table->count;
      cover_count = (int *)malloc(sizeof(int) * count);
      if (!cover_count) return 1;

      for (i = start_index; i < gen->size; i++) {
          double r = (double)rand() / (double)RAND_MAX;
          Individu *ind = &gen->items[i];
          int round;

          if (!ind->indices || ind->nb_hop <= 0) continue;
          if (ind->nb_hop > count) continue;
          if (r >= guided_rate) continue;

          /* cover_count[c] = nb d'hopitaux de l'individu qui couvrent la commune c */
          for (j = 0; j < count; j++) cover_count[j] = 0;
          for (j = 0; j < ind->nb_hop; j++) {
              int hi = ind->indices[j];
              if (hi < 0 || hi >= count) continue;
              for (k = coverage->offsets[hi]; k < coverage->offsets[hi + 1]; k++)
                  cover_count[coverage->indices[k]]++;
          }

          /* 1. AJOUTS gain-gardes : on ne pose un hopital que s'il couvre plus de
                5000 habitants encore en desert (sinon il coute plus qu'il rapporte). */
          for (round = 0; round < GUIDED_MAX_ROUNDS; round++) {
              int best_cand = -1;
              long best_gain = 0;
              int tries;
              if (ind->nb_hop >= count) break;

              for (tries = 0; tries < GUIDED_SAMPLE_TRIES; tries++) {
                  int c = rand() % count;
                  long gain = 0;
                  if (cover_count[c] != 0) continue; /* commune deja couverte */
                  for (k = coverage->offsets[c]; k < coverage->offsets[c + 1]; k++) {
                      int cc = coverage->indices[k];
                      if (cover_count[cc] == 0) gain += table->items[cc].population;
                  }
                  if (gain > best_gain) { best_gain = gain; best_cand = c; }
              }

              if (best_cand < 0 || best_gain <= (long)FITNESS_PENALTY_HOP) break;

              {
                  int *grown = (int *)realloc(ind->indices,
                                              sizeof(int) * (ind->nb_hop + 1));
                  if (!grown) break;
                  ind->indices = grown;
                  ind->indices[ind->nb_hop++] = best_cand;
                  for (k = coverage->offsets[best_cand]; k < coverage->offsets[best_cand +
  1]; k++)
                      cover_count[coverage->indices[k]]++;
              }
          }

          /* 2. ELAGAGE : on retire les hopitaux dont la couverture UNIQUE (communes
                qu'eux seuls couvrent) pese moins que la penalite de 5000. Les
                retirer fait GAGNER de la fitness (on economise les 5000). */
          j = 0;
          while (j < ind->nb_hop && ind->nb_hop > 1) {
              int hi = ind->indices[j];
              int unique_pop = 0;
              if (hi < 0 || hi >= count) { j++; continue; }
              for (k = coverage->offsets[hi]; k < coverage->offsets[hi + 1]; k++) {
                  int c = coverage->indices[k];
                  if (cover_count[c] == 1) unique_pop += table->items[c].population;
              }
              if (unique_pop < FITNESS_PENALTY_HOP) {
                  for (k = coverage->offsets[hi]; k < coverage->offsets[hi + 1]; k++)
                      cover_count[coverage->indices[k]]--;
                  ind->indices[j] = ind->indices[ind->nb_hop - 1];
                  ind->nb_hop--;
              } else {
                  j++;
              }
          }
      }

      free(cover_count);
      return 0;
  }




int ga_reseed_generation(Generation *gen,
                         const GAConfig *config,
                         const MunicipalityTable *table,
                         int keep_elites)
{
    Generation new_gen;
    int *elite_indices;
    int i;

    elite_indices = NULL;

    if (!gen || !config || !table || !gen->items) return 1;
    if (gen->size <= 0) return 1;
    if (config->nb_hop <= 0) return 1;
    if (config->nb_hop > (int)table->count) return 1;

    if (keep_elites < 0) keep_elites = 0;
    if (keep_elites > gen->size) keep_elites = gen->size;

    if (keep_elites > 0) {
        elite_indices = (int *)malloc(sizeof(int) * keep_elites);
        if (!elite_indices) return 1;
        if (selection_find_elites(gen, elite_indices, keep_elites) != 0) {
            free(elite_indices);
            return 1;
        }
    }


    for (i = 0; i < keep_elites; i++) {
        if (copy_individual(&gen->items[elite_indices[i]],
                            &new_gen.items[i]) != 0) {
            if (elite_indices) free(elite_indices);
            generation_free(&new_gen);
            return 1;
        }
    }

    generation_free(gen);
    *gen = new_gen;

    if (elite_indices) free(elite_indices);
    return 0;
}
