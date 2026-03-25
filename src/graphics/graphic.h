#ifndef GRAPHIC_H
#define GRAPHIC_H
#include "map/map.h"
#include <MLV/MLV_all.h>

void init_window(int width, int height);
void draw_map_of_towns(GraphicTownPoint* towns, int num_towns);
void draw_hospitals();
void close_window();
void actualise();

#endif
