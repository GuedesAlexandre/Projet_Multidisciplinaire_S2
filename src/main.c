#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "./genetic/generation.h"
#include "./data/municipality.h"
#include "./parsers/parser_csv.h"
#include "./genetic/fitness.h"
#include "./genetic/crossover.h"
#include "./genetic/selection.h"

#define NB_HOP        500
#define POP_SIZE      100
#define SOLUTION_CSV  "assets/best_solution_mock.csv"
/* Limite de latitude pour l'optimisation de la distance (évite haversine inutile) */
#define LAT_LIMIT     (HOSPITAL_RADIUS_KM / 111.0)

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

/* Retourne l'indice du meilleur individu dans la génération */
static int get_best_index(const Generation *gen)
{
    int i, bi = 0;
    for (i = 1; i < gen->size; i++)
        if (gen->items[i].fitness > gen->items[bi].fitness)
            bi = i;
    return bi;
}

/*
 * Exporte la solution du meilleur individu dans un CSV lu par generate_pdf.py.
 * Format : insee_code,a_hopital,est_chru,hopital_desservant
 * - a_hopital          : 1 si la commune est un hôpital dans la solution
 * - est_chru           : 1 si hôpital et population >= CHRU_POP_THRESHOLD
 * - hopital_desservant : insee_code du plus proche hôpital à <= 10 km, vide si désert
 */
static void export_solution(const MunicipalityTable *table,
                             const Individu *best,
                             const char *path)
{
    int *is_hospital;
    FILE *f;
    size_t i;
    int j;
    const Municipality *items = table->items;

    is_hospital = (int *)calloc(table->count, sizeof(int));
    if (!is_hospital) {
        fprintf(stderr, "export_solution: allocation echouee\n");
        return;
    }

    /* Marque les communes qui sont des hôpitaux */
    for (j = 0; j < best->nb_hop; j++) {
        int hi = best->indices[j];
        if (hi >= 0 && (size_t)hi < table->count)
            is_hospital[hi] = 1;
    }

    f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "export_solution: impossible d'ouvrir %s\n", path);
        free(is_hospital);
        return;
    }

    fprintf(f, "insee_code,a_hopital,est_chru,hopital_desservant\n");

    for (i = 0; i < table->count; i++) {
        int a_hop    = is_hospital[i];
        int est_chru = (a_hop && items[i].population >= CHRU_POP_THRESHOLD) ? 1 : 0;
        int nearest  = -1;
        double best_dist = HOSPITAL_RADIUS_KM + 1.0;

        /* Cherche le plus proche hôpital dans le rayon autorisé */
        for (j = 0; j < best->nb_hop; j++) {
            int hi = best->indices[j];
            double dist;
            if (hi < 0 || (size_t)hi >= table->count) continue;
            if (fabs(items[i].latitude - items[hi].latitude) > LAT_LIMIT) continue;
            dist = haversine_km(items[i].latitude, items[i].longitude,
                                items[hi].latitude, items[hi].longitude);
            if (dist < best_dist) {
                best_dist = dist;
                nearest   = hi;
            }
        }

        if (nearest >= 0)
            fprintf(f, "%d,%d,%d,%d\n",
                    items[i].insee_code, a_hop, est_chru,
                    items[nearest].insee_code);
        else
            fprintf(f, "%d,%d,%d,\n",
                    items[i].insee_code, a_hop, est_chru);
    }

    fclose(f);
    free(is_hospital);
    printf("Solution exportee : %s\n", path);
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
    printf("Communes chargees : %ld\n", (long)table.count);

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

    /* 8. Export de la meilleure solution et generation du PDF */
    {
        int bi = get_best_index(&children);
        int ret;
        export_solution(&table, &children.items[bi], SOLUTION_CSV);
        ret = system("python3 scripts/generate_pdf.py");
        if (ret != 0)
            fprintf(stderr, "Erreur lors de la generation du PDF (code %d)\n", ret);
    }

    /* Liberation memoire */
    generation_free(&gen);
    generation_free(&children);
    municipality_table_free(&table);

    return 0;
}