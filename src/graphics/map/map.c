#include "map.h"
#include <math.h>

#define LAT_CENTER 46.5f

int lon_to_x(float lon, int window_width) {
    float cos_factor = cos(LAT_CENTER * M_PI / 180.0f);
    return (int)((lon - LON_MIN) / (LON_MAX - LON_MIN) * window_width * cos_factor
                 + window_width * (1.0f - cos_factor) / 2.0f);
}

int lat_to_y(float lat, int window_height) {
    float cos_factor = cos(LAT_CENTER * M_PI / 180.0f);
    return (int)((lat - LAT_MIN) / (LAT_MAX - LAT_MIN) * window_height * cos_factor
                 + window_height * (1.0f - cos_factor) / 2.0f);
}
