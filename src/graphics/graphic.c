#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <MLV/MLV_all.h>

#include "graphic.h"
#include "map/map.h"

#include "../data/municipality.h"
#include "../parsers/parser_csv.h"
#include "../genetic/generation.h"
#include "../genetic/fitness.h"

/* Positions du texte d'information à l'écran */
#define TEXT_X            10
#define TEXT_LINE_1_Y     10
#define TEXT_LINE_2_Y     30

/* Paramètres d'animation */
#define ANIMATION_COUNT       10
#define ANIMATION_DELAY_MS    500

/* Taille des buffers pour les chaines */
#define LABEL_BUFFER_SIZE   128

void init_window(int width, int height) {
    MLV_create_window("Projet S2 - Déserts Médicaux", "Projet S2 - Déserts Médicaux", width, height);
    MLV_clear_window(MLV_COLOR_BLACK);
};


void draw_map_of_towns(Municipality* towns, int num_towns, MLV_Image* background) {
    MLV_draw_filled_rectangle(0, 0, MAP_WIDTH, MAP_HEIGHT, MLV_COLOR_BLACK);
    int i;
    for ( i = 0; i < num_towns; i++) {
        int x = lon_to_x(towns[i].longitude, MAP_WIDTH);
        int y = lat_to_y(towns[i].latitude, MAP_HEIGHT);
        MLV_draw_point_on_image(x, y, MLV_COLOR_BLUE, background);
    }

}


void draw_chru(Municipality* towns, int index) {
    int x = lon_to_x(towns[index].longitude, MAP_WIDTH);
    int y = lat_to_y(towns[index].latitude, MAP_HEIGHT);
    MLV_draw_filled_circle(x, y, 4, MLV_COLOR_GREEN);
}


void draw_hospitals_from_individu(Municipality* towns, int num_towns, Individu* ind) {
    int i;
    int index;
    /* Parcourt de tous les hôpitaux prévus par cet individu (struct) */
    for(i = 0; i < ind->nb_hop; i++) {
        /* Tableau d'indices de l'individu (numéros des villes choisies pour accueillir un hôpital) ex: towns[45] */
        index = ind->indices[i];
        if(towns[index].population >= CHRU_POP_THRESHOLD) {
            draw_chru(towns, index);
        } else {
            int x = lon_to_x(towns[index].longitude, MAP_WIDTH);
            int y = lat_to_y(towns[index].latitude, MAP_HEIGHT);
            MLV_draw_filled_circle(x, y, 1, MLV_COLOR_RED);
        }
    }
}

int generation_get_best_index(const Generation* gen) {
    int i;
    int best_index = 0;
    double best_fitness;

    best_fitness = gen->items[0].fitness;

    for (i = 1; i < gen->size; i++) {
        if (gen->items[i].fitness > best_fitness) {
            best_fitness = gen->items[i].fitness;
            best_index = i;
        }
    }
    return best_index;
}


void draw_individu_info(Individu* ind, long habitants_desert, const char* title, MLV_Color title_color) {
    char stats[LABEL_BUFFER_SIZE];
    MLV_draw_text(TEXT_X, TEXT_LINE_1_Y, title, title_color);
    sprintf(stats, "Fitness: %.0f | CHRU: %d | Habitants en desert: %ld",
            ind->fitness, ind->nb_chru, habitants_desert);
    MLV_draw_text(TEXT_X, TEXT_LINE_2_Y, stats, MLV_COLOR_WHITE);
}


void display_genetic_results(MunicipalityTable* table, Generation* gen, long* habitants_desert_by_individu) {
    int i;
    
    init_window(WINDOW_WIDTH, WINDOW_HEIGHT);
    MLV_Image* bg = MLV_create_image(WINDOW_WIDTH, WINDOW_HEIGHT);
    draw_map_of_towns(table->items, table->count, bg);
    int best_index = generation_get_best_index(gen);
    
    char title[LABEL_BUFFER_SIZE];

    /* Affichage des premiers individus */
    for (i = 0; i < ANIMATION_COUNT; i++) {
        MLV_draw_image(bg, 0, 0);
        draw_hospitals_from_individu(table->items, table->count, &gen->items[i]);
        sprintf(title, "Individu %d / %d", i + 1, gen->size);
        draw_individu_info(&gen->items[i], habitants_desert_by_individu[i], title, MLV_COLOR_WHITE);
        actualise();
        MLV_wait_milliseconds(ANIMATION_DELAY_MS);
    }
    
    /* Affichage final avec le meilleur individu */
    MLV_draw_image(bg, 0, 0);
    draw_hospitals_from_individu(table->items, table->count, &gen->items[best_index]);
    sprintf(title, "LE MEILLEUR INDIVIDU (index %d)", best_index);
    draw_individu_info(&gen->items[best_index], habitants_desert_by_individu[best_index], title, MLV_COLOR_GREEN);
    actualise();
    
    MLV_wait_mouse(NULL, NULL);
    MLV_free_image(bg);
    close_window();
}

void close_window() {
    MLV_free_window();
}


void actualise(){
    /* 
    TODO: Ajouter méthode pour réinitialiser la struct avec données de nouvelle génération.
    */
    MLV_actualise_window();
}