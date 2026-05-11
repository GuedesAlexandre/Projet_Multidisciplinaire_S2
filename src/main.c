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

    init_window(WINDOW_WIDTH, WINDOW_HEIGHT);
    MLV_Image* bg = MLV_create_image(WINDOW_WIDTH, WINDOW_HEIGHT);
    draw_map_of_towns(table.items, table.count, bg);
    
    int best_index = 0;
    double best_fitness = gen.items[0].fitness;
    for(i = 1; i < gen.size; i++) {
        if(gen.items[i].fitness > best_fitness) {
            best_fitness = gen.items[i].fitness;
            best_index = i;
        }
    }
    
    char label[128];
    char stats[128];
    
    for(i = 0; i < 10; i++) {
        MLV_draw_image(bg, 0, 0);
        draw_hospitals_from_individu(table.items, table.count, &gen.items[i]); /* adresse de la structure Individy */
        
        sprintf(label, "Individu %d / %d", i + 1, gen.size);
        MLV_draw_text(10, 10, label, MLV_COLOR_WHITE);
        sprintf(stats, "Fitness: %.0f | CHRU: %d | Habitants en desert: %ld", 
                gen.items[i].fitness, gen.items[i].nb_chru, habitants_desert_by_individu[i]);
        MLV_draw_text(10, 30, stats, MLV_COLOR_WHITE);
        
        actualise();
        MLV_wait_milliseconds(500);
    }
    
    MLV_draw_image(bg, 0, 0);
    draw_hospitals_from_individu(table.items, table.count, &gen.items[best_index]);
    
    MLV_draw_text(10, 10, "LE MEILLEUR INDIVIDU :", MLV_COLOR_GREEN);
    sprintf(stats, "Index %d | Fitness: %.0f | CHRU: %d | Habitants en desert: %ld", 
            best_index, gen.items[best_index].fitness, 
            gen.items[best_index].nb_chru, habitants_desert_by_individu[best_index]);
    MLV_draw_text(10, 30, stats, MLV_COLOR_WHITE);
    
    actualise();
    MLV_wait_mouse(NULL, NULL);
    
    MLV_free_image(bg);
    close_window();

    free(habitants_desert_by_individu);
    generation_free(&gen);
    municipality_table_free(&table);

    return 0;
}
