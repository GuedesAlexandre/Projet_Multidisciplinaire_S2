#include <stdio.h>
#include <stdlib.h>
#include <MLV/MLV_all.h>
#include <time.h>

#include "genetic/generation.h"
#include "data/municipality.h"
#include "parsers/parser_csv.h"
#include "genetic/fitness.h"
#include "graphics/graphic.h"

#define CSV_PATH "assets/communes-france-metrople-2025.csv"

int main() {
  /* seed (graine) pour rendre vraiment aléatoire (calcule à partir de la seed)*/
    srand(time(NULL));
    MunicipalityTable table;
    Generation gen;
    long* habitants_desert_by_individu;
    int i;
    long habitants_desert;
    int nb_chru;

    csv_parser_parse_file(CSV_PATH, &table);

    generation_init_random(&gen, 100, 500, 34347);
    habitants_desert_by_individu = (long*)malloc(sizeof(long) * gen.size);
    if (!habitants_desert_by_individu) {
        generation_free(&gen);
        municipality_table_free(&table);
        return 1;
    }

    for(i=0; i<gen.size; i++) {
      gen.items[i].fitness=compute_fitness(&table, gen.items[i].indices, 500, &habitants_desert, &nb_chru);
      gen.items[i].nb_chru=nb_chru;
      habitants_desert_by_individu[i]=habitants_desert;
    }

    for(i=0; i<gen.size; i++) {
      printf("Individu %d: fitness = %f, nb_chru = %d, habitants_desert = %ld\n", i, gen.items[i].fitness, gen.items[i].nb_chru, habitants_desert_by_individu[i]);
    }

    display_genetic_results(&table, &gen, habitants_desert_by_individu);

    free(habitants_desert_by_individu);
    generation_free(&gen);
    municipality_table_free(&table);

    return 0;
}
