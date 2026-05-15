#include "fitness.h"
#include <math.h>

#define PI 3.14159265358979323846
#define LAT_DIFF_LIMIT_DEG (HOSPITAL_RADIUS_KM / 111.0)

static double deg_to_rad(double degree_value) {
    return degree_value * (PI / 180.0);
}

/* Formule trouvé sur internet et juste appliqué en c */
double haversine_km(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371.0;
    double dlat = deg_to_rad(lat2 - lat1);
    double dlon = deg_to_rad(lon2 - lon1);
    double a = sin(dlat / 2.0) * sin(dlat / 2.0)
             + cos(deg_to_rad(lat1)) * cos(deg_to_rad(lat2))
             * sin(dlon / 2.0) * sin(dlon / 2.0);
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    return R * c;
}



double compute_fitness(const MunicipalityTable* table,
                       const int* hospital_indices,
                       int hospital_count,
                       long* out_habitants_desert,
                       int* out_nb_chru) {
    size_t i;
    int j;
    const Municipality* items = table->items;
    long habitants_desert = 0;
    int nb_chru = 0;

    for (j = 0; j < hospital_count; j++) {
        int hi = hospital_indices[j];
        if (hi >= 0 && (size_t)hi < table->count &&
            items[hi].population >= CHRU_POP_THRESHOLD) {
            nb_chru++;
        }
    }

    for (i = 0; i < table->count; i++) {
        int covered = 0;
        double lat = items[i].latitude;
        double lon = items[i].longitude;

        for (j = 0; j < hospital_count; j++) {
            int hi = hospital_indices[j];
            double hlat;
            if (hi < 0 || (size_t)hi >= table->count) continue;

            hlat = items[hi].latitude;
            if (fabs(lat - hlat) > LAT_DIFF_LIMIT_DEG) continue;

            if (haversine_km(lat, lon,
                             hlat,
                             items[hi].longitude) < HOSPITAL_RADIUS_KM) {
                covered = 1;
                break;
            }
        }

        if (!covered) {
            habitants_desert += items[i].population;
        }
    }

    if (out_habitants_desert) *out_habitants_desert = habitants_desert;
    if (out_nb_chru) *out_nb_chru = nb_chru;

    return (double)FITNESS_BASE_POPULATION
         - (double)habitants_desert
         - (double)(FITNESS_PENALTY_HOP * hospital_count)
         + (double)(FITNESS_BONUS_CHRU * nb_chru);
}
