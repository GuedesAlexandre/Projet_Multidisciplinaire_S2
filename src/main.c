#include <stdio.h>
#include "./genetic/generation.h"
#include "./data/municipality.h"
#include "./parsers/parser_csv.h"
#include "./genetic/fitness.h"

int main() {
    MunicipalityTable table;
    csv_parser_parse_file("./assets/communes-france-metrople-2025.csv", &table);
    Generation gen;
    generation_init_random(&gen, 100, 500, 34347);
    int i;
    long habitants_desert;
    int nb_chru;
    for(i=0; i<gen.size; i++) {
     gen.items[i].fitness=compute_fitness(&table, gen.items[i].indices, 500, &habitants_desert, &nb_chru);
        gen.items[i].nb_chru=nb_chru;
    }
    for(i=0; i<gen.size; i++) {
      printf("Individu %d: fitness = %f, nb_chru = %d, habitants_desert = %ld\n", i, gen.items[i].fitness, gen.items[i].nb_chru, habitants_desert);
    }
    generation_free(&gen);
    municipality_table_free(&table);
     return 0;
}
