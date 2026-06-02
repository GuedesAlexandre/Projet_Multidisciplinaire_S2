#include "csv_export.h"
#include "../genetic/fitness.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int csv_export_best_solution(const char *output_path,
                             const MunicipalityTable *table,
                             const Individu *best)
{
    FILE *f;
    int *is_hospital;
    size_t i;
    int j;
    double lat_diff_limit;

    if (!output_path || !table || !best || !best->indices)
        return 1;

    is_hospital = (int *)calloc(table->count, sizeof(int));
    if (!is_hospital)
        return 1;

    for (j = 0; j < best->nb_hop; j++) {
        int idx = best->indices[j];
        if (idx >= 0 && (size_t)idx < table->count)
            is_hospital[idx] = 1;
    }

    f = fopen(output_path, "w");
    if (!f) {
        free(is_hospital);
        return 1;
    }
 
    lat_diff_limit = HOSPITAL_RADIUS_KM / 111.0;

    fprintf(f, "insee_code,a_hopital,est_chru,hopital_desservant\n");

    for (i = 0; i < table->count; i++) {
        int a_hopital = is_hospital[i];
        int est_chru  = (a_hopital && table->items[i].population >= CHRU_POP_THRESHOLD) ? 1 : 0;
        int serving   = 0;
        double lat_i  = table->items[i].latitude;
        double lon_i  = table->items[i].longitude;

        if (a_hopital) {
            serving = table->items[i].insee_code;
        } else {
            double min_dist = -1.0;
            for (j = 0; j < best->nb_hop; j++) {
                int hi = best->indices[j];
                double dist;
                if (hi < 0 || (size_t)hi >= table->count) continue;
                if (fabs(lat_i - table->items[hi].latitude) > lat_diff_limit) continue;
                dist = haversine_km(lat_i, lon_i,
                                    table->items[hi].latitude,
                                    table->items[hi].longitude);
                if (dist < HOSPITAL_RADIUS_KM) {
                    if (min_dist < 0.0 || dist < min_dist) {
                        min_dist = dist;
                        serving  = table->items[hi].insee_code;
                    }
                }
            }
        }

        if (serving != 0) {
            fprintf(f, "%d,%d,%d,%d\n",
                    table->items[i].insee_code, a_hopital, est_chru, serving);
        } else {
            fprintf(f, "%d,%d,%d,\n",
                    table->items[i].insee_code, a_hopital, est_chru);
        }
    }

    fclose(f);
    free(is_hospital);
    return 0;
}
