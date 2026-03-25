#ifndef MAP_H
#define MAP_H

/* Coordonnées  nord sud ouest est */
#define LAT_MAX  51.1f
#define LAT_MIN  41.3f
#define LON_MIN  -5.2f
#define LON_MAX   9.6f

/* Structure pour les coordonnées graphiques */
typedef struct {
    float lat_center;
    float lon_center;
    float zoom;
} Graphic;

typedef struct {
    float lat;
    float lon;
    char name[100];
} GraphicTownPoint;

int lon_to_x(float lon, int window_width);
int lat_to_y(float lat, int window_height);

#endif
