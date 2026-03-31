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
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

#define CSV_PATH "assets/communes-france-metrople-2025.csv"

void init_window(int width, int height){
    MLV_create_window("Projet S2 - Déserts Médicaux", "Projet S2 - Déserts Médicaux", width, height);
    MLV_clear_window(MLV_COLOR_BLACK);
};


void draw_map_of_towns(Municipality* towns, int num_towns){
  int i;
    for ( i = 0; i < num_towns; i++) {
        int x = lon_to_x(towns[i].longitude, WINDOW_WIDTH);
        int y = lat_to_y(towns[i].latitude, WINDOW_HEIGHT);
        MLV_draw_point(x, y, MLV_COLOR_BLUE);
    }
}

void draw_hospitals(Municipality* towns, int num_towns, int num_hospitals){
    int i;
    int index;
    for( i = 0; i < num_hospitals; i++) {
        index = rand() % num_towns;
        int x = lon_to_x(towns[index].longitude, WINDOW_WIDTH);
        int y = lat_to_y(towns[index].latitude, WINDOW_HEIGHT);
        MLV_draw_point(x, y, MLV_COLOR_RED);
    }
}

void run_fake_generations(Municipality* towns, int num_towns, int num_generations, int hospitals_per_gen){
    int gen;
    char label[64];
    srand(time(NULL));
    for(gen = 0; gen < num_generations; gen++){
        MLV_clear_window(MLV_COLOR_BLACK);
        draw_map_of_towns(towns, num_towns);

        draw_hospitals(towns, num_towns, hospitals_per_gen);

        sprintf(label, "Generation %d / %d", gen + 1, num_generations);
        MLV_draw_text(10, 10, label, MLV_COLOR_WHITE);
        actualise();
        MLV_wait_milliseconds(500);
    }
}


void close_window(){
    MLV_free_window();
}


void actualise(){
    /* 
      TODO: Ajouter méthode pour réinitialiser la struct avec données de nouvelle génération.
    */
    MLV_actualise_window();
}

int main(int argc, char *argv[]) {
    MunicipalityTable table = {0};
    CsvParserError err;

    err = csv_parser_parse_file(CSV_PATH, &table);
    if (err != CSV_PARSER_OK) {
        fprintf(stderr, "Erreur CSV : %s\n", csv_error_mapper(err));
        return 1;
    }

    printf("%lu communes chargées.\n", (unsigned long)table.count);

    init_window(WINDOW_WIDTH, WINDOW_HEIGHT);
    draw_map_of_towns(table.items, (int)table.count);
    actualise();
    run_fake_generations(table.items, (int)table.count, 20, 5000);

    MLV_wait_mouse(NULL, NULL);
    close_window();
    municipality_table_free(&table);

    return 0;
}