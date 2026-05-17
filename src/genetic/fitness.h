#ifndef FITNESS_H
#define FITNESS_H

#include "../data/municipality.h"

#define HOSPITAL_RADIUS_KM      10.0
#define CHRU_POP_THRESHOLD      80000
#define FITNESS_BASE_POPULATION 65141355
#define FITNESS_PENALTY_HOP     5000
#define FITNESS_BONUS_CHRU      4000

double haversine_km(double lat1, double lon1, double lat2, double lon2);

double compute_fitness(const MunicipalityTable* table,
                       const int* hospital_indices,
                       int hospital_count,
                       long* out_habitants_desert,
                       int* out_nb_chru);

#endif
