#ifndef GRAPHIC_H
#define GRAPHIC_H
#include "map/map.h"
#include "municipality.h"
#include <MLV/MLV_all.h>

void init_window(int width, int height);
void draw_map_of_towns(Municipality* towns, int num_towns);
void draw_hospitals(Municipality* towns, int num_towns, int num_hospitals);
void run_fake_generations(Municipality* towns, int num_towns, int num_generations, int hospitals_per_gen);
void close_window();
void actualise();

#endif
