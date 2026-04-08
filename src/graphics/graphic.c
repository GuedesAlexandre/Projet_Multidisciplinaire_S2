#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <MLV/MLV_all.h>

#include "graphic.h"
#include "map.h"

#include "municipality.h"
#include "parser_csv.h"

#define LAT_CENTER 46.5f
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800
#define MAP_WIDTH 800
#define MAP_HEIGHT 800

#define CSV_PATH "assets/communes-france-metrople-2025.csv"

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

void draw_hospitals(Municipality* towns, int num_towns, int num_hospitals) {
    int i;
    int index;
    for( i = 0; i < num_hospitals; i++) {
        index = rand() % num_towns;
        if(towns[index].population > 80000) {
            draw_chru(towns, index);
        } else {
            int x = lon_to_x(towns[index].longitude, MAP_WIDTH);
            int y = lat_to_y(towns[index].latitude, MAP_HEIGHT);
            MLV_draw_point(x, y, MLV_COLOR_RED);
        }
    }
}

void draw_chru(Municipality* towns, int index) {
    int x = lon_to_x(towns[index].longitude, MAP_WIDTH);
    int y = lat_to_y(towns[index].latitude, MAP_HEIGHT);
    MLV_draw_filled_circle(x, y, 3, MLV_COLOR_GREEN);
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