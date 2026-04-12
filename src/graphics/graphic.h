#ifndef GRAPHIC_H
#define GRAPHIC_H

/**
 * @file graphic.h
 * @brief Header file for graphical display functions related to Municipality data.
 */
#include "map/map.h"
#include "municipality.h"
#include <MLV/MLV_all.h>

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
 * @brief Closes the graphical window and releases associated resources.
 */
void close_window();

/**
 * @brief Refreshes the graphical window display.
 */
void actualise();

#endif
