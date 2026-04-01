#ifndef GRAPHIC_H
#define GRAPHIC_H
#include "map/map.h"
#include "municipality.h"
#include <MLV/MLV_all.h>

void init_window(int width, int height);
void draw_map_of_towns(Municipality* towns, int num_towns, MLV_Image* background);
void draw_hospitals(Municipality* towns, int num_towns, int num_hospitals);
void draw_chru(Municipality* towns, int index);
void run_fake_generations(Municipality* towns, int num_towns, int num_generations, int hospitals_per_gen, MLV_Image* background);
void close_window();
void actualise();

#endif
