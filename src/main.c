#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "./genetic/ga.h"
#include "./genetic/coverage.h"
#include "./parsers/parser_csv.h"

static void print_generation_summary(int generation_index,
                                     const MunicipalityTable *table,
                                     const CoverageIndex *coverage,
                                     const Generation *gen,
                                     double mutation_rate)
{
    double best_fitness = 0.0;
    long habitants_desert = 0;
    int nb_chru = 0;

    ga_get_best_stats(table, coverage, gen, &best_fitness, &habitants_desert, &nb_chru);

    printf("Gen %d | Best = %.0f | Avg = %.0f | Desert = %ld | CHRU = %d | Mut = %.3f\n",
           generation_index,
           best_fitness,
           ga_get_avg_fitness(gen),
           habitants_desert,
           nb_chru,
           mutation_rate);
}

int main(void)
{
    int generation;
    int stagnation;
    int restart_count;
    double best_overall;
    double current_best;
    double immigrants_base;
    MunicipalityTable table;
    Generation gen;
    Generation next;
    GAConfig config;
    CoverageIndex coverage;

    srand(time(NULL));

    /* Chargement des communes */
    csv_parser_parse_file("./assets/communes-france-metrople-2025.csv", &table);
    printf("Communes chargees : %d\n", (int)table.count);

    if (coverage_build(&table, &coverage) != 0) {
        printf("Erreur construction index couverture\n");
        municipality_table_free(&table);
        return 1;
    }

    config = ga_config_default();
    printf("Config GA: pop=%d, hop=%d, max_gen=%d, elite=%d, tournoi=%d, mutation=%.2f->%.2f, immigrants=%.2f<=%.2f, restart=%d, max_restart=%d, cible=%.0f\n",
           config.population_size,
           config.nb_hop,
           config.max_generations,
           config.elite_count,
           config.tournament_k,
           config.mutation_rate_start,
           config.mutation_rate_end,
           config.immigrants_rate,
           config.immigrants_rate_max,
           config.restart_threshold,
           config.max_restarts,
           config.target_fitness);

    if (ga_init_generation(&gen, &config, &table) != 0) {
        printf("Erreur generation initiale\n");
        municipality_table_free(&table);
        return 1;
    }

    ga_evaluate_generation(&table, &coverage, &gen);

    best_overall = ga_get_best_fitness(&gen);
    if (config.verbose)
        print_generation_summary(0, &table, &coverage, &gen, config.mutation_rate);

    stagnation = 0;
    restart_count = 0;
    immigrants_base = config.immigrants_rate;

    if (best_overall >= config.target_fitness) {
        printf("Objectif deja atteint: %.0f\n", best_overall);
    } else {
        for (generation = 1; generation <= config.max_generations; generation++) {
            double progress;
            double base_rate;
            double boost = 0.0;
            double immigrants_rate;

            if (config.max_generations > 1)
                progress = (double)(generation - 1) / (double)(config.max_generations - 1);
            else
                progress = 1.0;

            base_rate = config.mutation_rate_start
                      + (config.mutation_rate_end - config.mutation_rate_start) * progress;

            if (stagnation >= 10) boost = 0.02;
            if (stagnation >= 30) boost = 0.05;

            config.mutation_rate = base_rate + boost;
            if (config.mutation_rate < config.mutation_rate_end)
                config.mutation_rate = config.mutation_rate_end;
            if (config.mutation_rate > 0.35)
                config.mutation_rate = 0.35;

            immigrants_rate = immigrants_base;
            if (stagnation >= 10) immigrants_rate += 0.05;
            if (stagnation >= 30) immigrants_rate += 0.15;
            if (stagnation >= 60) immigrants_rate += 0.25;
            if (immigrants_rate > config.immigrants_rate_max)
                immigrants_rate = config.immigrants_rate_max;
            config.immigrants_rate = immigrants_rate;

            next = ga_next_generation(&gen, &config, (int)table.count);
            if (next.size == 0) {
                printf("Erreur generation suivante\n");
                break;
            }

            ga_evaluate_generation(&table, &coverage, &next);

            current_best = ga_get_best_fitness(&next);
            if (current_best > best_overall) {
                best_overall = current_best;
                stagnation = 0;
            } else {
                stagnation++;
            }

            if (config.verbose)
                print_generation_summary(generation, &table, &coverage, &next, config.mutation_rate);

            generation_free(&gen);
            gen = next;

            if (current_best >= config.target_fitness) {
                printf("Objectif atteint: %.0f\n", current_best);
                break;
            }

            if (config.restart_threshold > 0 && stagnation >= config.restart_threshold) {
                restart_count++;
                printf("Restart %d/%d a la generation %d (stagnation=%d)\n",
                       restart_count, config.max_restarts, generation, stagnation);

                if (config.max_restarts > 0 && restart_count > config.max_restarts) {
                    printf("Arret: nombre maximum de restarts atteint\n");
                    break;
                }

                if (ga_reseed_generation(&gen, &config, &table, config.elite_count) != 0) {
                    printf("Erreur reseed\n");
                    break;
                }

                ga_evaluate_generation(&table, &coverage, &gen);
                current_best = ga_get_best_fitness(&gen);
                if (current_best > best_overall) best_overall = current_best;
                stagnation = 0;

                if (config.verbose)
                    print_generation_summary(generation, &table, &coverage, &gen, config.mutation_rate);

                continue;
            }

            if (config.stagnation_limit > 0 && stagnation >= config.stagnation_limit) {
                printf("Arret pour stagnation (%d generations)\n",
                       config.stagnation_limit);
                break;
            }
        }
    }

    printf("Meilleur fitness final: %.0f\n", best_overall);

    generation_free(&gen);
    coverage_free(&coverage);
    municipality_table_free(&table);

    return 0;
}
