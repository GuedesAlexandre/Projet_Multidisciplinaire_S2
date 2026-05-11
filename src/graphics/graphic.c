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
    MLV_draw_filled_circle(x, y, 3, MLV_COLOR_GREEN);
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
            MLV_draw_point(x, y, MLV_COLOR_RED);
        }
    }
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