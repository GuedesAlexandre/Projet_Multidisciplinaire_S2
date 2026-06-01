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


void display_generation(MLV_Image *bg, const MunicipalityTable *table, const Generation *gen, int gen_index, double avg_fitness, long total_population) {
    int best_index;
    Individu *best;
    int tx;
    int y;
    char buf[LABEL_BUFFER_SIZE];
    double pct_hab;
    double pct_com;

    best_index = generation_get_best_index(gen);
    best = &gen->items[best_index];

    pct_hab = (total_population > 0) ? (double)best->habitants_desert / (double)total_population * 100.0 : 0.0;
    pct_com = (table->count > 0) ? (double)best->communes_desert / (double)table->count * 100.0 : 0.0;

    tx = MAP_WIDTH + 10;
    y  = 10;

    /* Fond de carte et hôpitaux du meilleur individu */
    MLV_draw_image(bg, 0, 0);
    MLV_draw_filled_rectangle(MAP_WIDTH, 0, WINDOW_WIDTH - MAP_WIDTH, WINDOW_HEIGHT, MLV_COLOR_BLACK);
    draw_hospitals_from_individu(table->items, (int)table->count, best);

    /* Numéro de génération */
    sprintf(buf, "Generation %d", gen_index);
    MLV_draw_text(tx, y, buf, MLV_rgba(255, 220, 0, 255));
    y += 30;

    /* Hôpitaux */
    sprintf(buf, "%d Hopitaux", best->nb_hop);
    MLV_draw_text(tx, y, buf, MLV_COLOR_WHITE);
    y += 20;

    sprintf(buf, "%d C.H.R.U.", best->nb_chru);
    MLV_draw_text(tx, y, buf, MLV_rgba(100, 200, 255, 255));
    y += 35;

    /* Désert population */
    sprintf(buf, "%ld Habitants eloignes", best->habitants_desert);
    MLV_draw_text(tx, y, buf, MLV_COLOR_WHITE);
    y += 20;

    sprintf(buf, "%.2f%%%% de la pop en desert", pct_hab);
    MLV_draw_text(tx, y, buf, MLV_COLOR_WHITE);
    y += 35;

    /* Désert communes */
    sprintf(buf, "%ld Villes eloignees", best->communes_desert);
    MLV_draw_text(tx, y, buf, MLV_COLOR_WHITE);
    y += 20;

    sprintf(buf, "%.2f%%%% de communes eloignees", pct_com);
    MLV_draw_text(tx, y, buf, MLV_COLOR_WHITE);
    y += 35;

    /*  Fitness */
    sprintf(buf, "FITNESS %.0f", best->fitness);
    MLV_draw_text(tx, y, buf, MLV_rgba(100, 255, 100, 255));
    y += 35;

    /* Stats de la génération */
    sprintf(buf, "Taille Population : %d", gen->size);
    MLV_draw_text(tx, y, buf, MLV_COLOR_WHITE);
    y += 20;

    sprintf(buf, "Fitness moyenne : %.0f", avg_fitness);
    MLV_draw_text(tx, y, buf, MLV_COLOR_WHITE);

    actualise();
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
