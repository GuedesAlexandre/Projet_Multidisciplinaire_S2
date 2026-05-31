#ifndef GRAPHIC_H
#define GRAPHIC_H

/**
 * @file graphic.h
 * @brief Header file for graphical display functions related to Municipality data.
 */
#include "map/map.h"
#include "../data/municipality.h"
#include "../genetic/generation.h"
#include <MLV/MLV_all.h>

#define LAT_CENTER 46.5f
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800
#define MAP_WIDTH 800
#define MAP_HEIGHT 800

/**
 * @brief Initializes the graphical window.
 * @param width The width of the window in pixels.
 * @param height The height of the window in pixels.
 */
void init_window(int width, int height);

/**
 * @brief Draws the map of towns on a background image.
 * @param towns The array of municipalities to display.
 * @param num_towns The number of municipalities in the array.
 * @param background The image used as drawing support.
 */
void draw_map_of_towns(Municipality* towns, int num_towns, MLV_Image* background);

/**
 * @brief Draws a municipality as a CHRU.
 * @param towns The array of municipalities.
 * @param index The index of the municipality in the array to be drawn as a CHRU.
 */
void draw_chru(Municipality* towns, int index);

/**
 * @brief Draws all hospitals of a specific genetic individual.
 * @details Regular hospitals are drawn in red.
 * @param towns The array of municipalities.
 * @param num_towns The total number of municipalities in the array.
 * @param ind Pointer to the  current individu.
 */
void draw_hospitals_from_individu(Municipality* towns, int num_towns, Individu* ind);

/**
 * @brief Get the index of the best generation.
 * @param gen The generation containing the evaluated individuals.
 */
int generation_get_best_index(const Generation* gen);

/**
 * @brief Affiche l'état d'une génération : carte du meilleur individu + statistiques.
 * @param bg         Image de fond (fond de carte sans hôpitaux).
 * @param table      Table des communes.
 * @param gen        Génération courante (évaluée).
 * @param gen_index  Numéro de la génération (pour l'affichage).
 * @param avg_fitness Fitness moyenne de la génération.
 * @param total_population Population totale du territoire (pour le pourcentage).
 */
void display_generation(MLV_Image *bg,
                        const MunicipalityTable *table,
                        const Generation *gen,
                        int gen_index,
                        double avg_fitness,
                        long total_population);

/**
 * @brief Closes the graphical window and releases associated resources.
 */
void close_window();

/**
 * @brief Refreshes the graphical window display.
 */
void actualise();

#endif
