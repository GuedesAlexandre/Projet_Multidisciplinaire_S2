#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "./genetic/generation.h"
#include "./data/municipality.h"
#include "./parsers/parser_csv.h"
#include "./genetic/fitness.h"
#include "./genetic/crossover.h"
#include "./genetic/selection.h"

#define NB_HOP   500
#define POP_SIZE 100

static void compute_fitness_gen(const MunicipalityTable *table, Generation *gen)
{
    int i;
    long habitants_desert;
    int nb_chru;
    for (i = 0; i < gen->size; i++) {
        gen->items[i].fitness = compute_fitness(table, gen->items[i].indices,
                                                NB_HOP, &habitants_desert, &nb_chru);
        gen->items[i].nb_chru = nb_chru;
    }
}

static double get_best(const Generation *gen)
{
    int i;
    double best = 0.0;
    for (i = 0; i < gen->size; i++)
        if (gen->items[i].fitness > best) best = gen->items[i].fitness;
    return best;
}

static double get_avg(const Generation *gen)
{
    int i;
    double avg = 0.0;
    for (i = 0; i < gen->size; i++) avg += gen->items[i].fitness;
    return avg / gen->size;
}

static void print_gen(const Generation *gen, const char *label)
{
    int i;
    printf("\n=== %s (%d individus) ===\n", label, gen->size);
    for (i = 0; i < gen->size; i++)
        printf("Individu %d: fitness = %.0f, nb_chru = %d\n",
               i, gen->items[i].fitness, gen->items[i].nb_chru);
    printf(">> Meilleure = %.0f | Moyenne = %.0f\n",
           get_best(gen), get_avg(gen));
}

static void inject_elites(Generation *children, const Generation *gen,
                           const int *elites)
{
    int i, j, last;
    for (i = 0; i < ELITE_COUNT; i++) {
        last = children->size - 1 - i;
        free(children->items[last].indices);
        children->items[last].indices = (int *)malloc(sizeof(int) * NB_HOP);
        if (!children->items[last].indices) continue;
        for (j = 0; j < NB_HOP; j++)
            children->items[last].indices[j] = gen->items[elites[i]].indices[j];
        children->items[last].fitness = gen->items[elites[i]].fitness;
        children->items[last].nb_chru = gen->items[elites[i]].nb_chru;
        children->items[last].nb_hop  = NB_HOP;
    }
}

int main()
{
    int i;
    MunicipalityTable table;
    Generation gen;
    Generation children;
    int elites[ELITE_COUNT];

    srand(time(NULL));

    /* Chargement des communes */
    csv_parser_parse_file("./assets/communes-france-metrople-2025.csv", &table);
    printf("Communes chargees : %d\n", table.count);

    /* 1. Generation initiale */
    generation_init_random(&gen, POP_SIZE, NB_HOP, table.count);

    /* 2. Calcul fitness */
    compute_fitness_gen(&table, &gen);
    print_gen(&gen, "Generation initiale");

    /* 3. Identification des elites */
    for (i = 0; i < ELITE_COUNT; i++) elites[i] = -1;
    if (selection_find_elites(&gen, elites, ELITE_COUNT) == 0) {
        printf("\n=== Elites ===\n");
        for (i = 0; i < ELITE_COUNT; i++)
            printf("Elite %d : individu %d, fitness = %.0f\n",
                   i, elites[i], gen.items[elites[i]].fitness);
    }

    /* 4. Crossover avec selection par tournoi */
    children = crossover_generation_new(&gen, NB_HOP);
    if (children.size == 0) {
        printf("Erreur crossover\n");
        generation_free(&gen);
        municipality_table_free(&table);
        return 1;
    }

    /* 5. Calcul fitness des enfants */
    compute_fitness_gen(&table, &children);
    print_gen(&children, "Generation apres crossover");

    /* 6. Injection des elites */
    inject_elites(&children, &gen, elites);
    printf("\n=== Apres injection des elites ===\n");
    printf("Meilleure = %.0f | Moyenne = %.0f\n",
           get_best(&children), get_avg(&children));

    /* 7. Comparaison finale */
    printf("\n=== Comparaison ===\n");
    printf("Generation initiale : meilleure = %.0f | moyenne = %.0f\n",
           get_best(&gen), get_avg(&gen));
    printf("Apres crossover     : meilleure = %.0f | moyenne = %.0f\n",
           get_best(&children), get_avg(&children));

    /* Liberation memoire */
    generation_free(&gen);
    generation_free(&children);
    municipality_table_free(&table);

    return 0;
}