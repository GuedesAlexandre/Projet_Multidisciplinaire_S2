#include "map.h"
#include <math.h>

#define LAT_CENTER 46.5f
#define PI 3.14159265358979323846f


/**
 * @brief Convertit les coordonnées géographiques en coordonnées graphiques
 * 
 * Cette fonction utilise une projection cartographique de type Mercator pour convertir
 * les coordonnées géographiques (latitude et longitude) en coordonnées graphiques (x et y).
 * La projection Mercator est couramment utilisée pour les cartes en ligne.
 * 
 * @details
 * Formules appliquées :
 * - Longitude vers X : x = (lon - LON_MIN) / (LON_MAX - LON_MIN) * window_width la formule de base pour la conversion de longitude en coordonnées x, où LON_MIN et LON_MAX définissent les limites de longitude de la carte, et window_width est la largeur de la fenêtre d'affichage en pixels.
 * - Latitude vers Y : y = (LAT_MAX - lat) / (LAT_MAX - LAT_MIN) * window_height la formule de base pour la conversion de latitude en coordonnées y, où LAT_MAX et LAT_MIN définissent les limites de latitude de la carte, et window_height est la hauteur de la fenêtre d'affichage en pixels.
 * 
 * Un facteur de correction basé sur la latitude centrale (LAT_CENTER) est appliqué
 * pour compenser la distorsion inhérente à la projection Mercator. Ce facteur utilise
 * la fonction cos() pour ajuster les coordonnées x et y, permettant une représentation
 * plus précise des distances sur la carte.
 * 
 * @note La projection Mercator préserve les angles et les formes, ce qui la rend
 *       idéale pour les applications de cartographie en ligne.
 * 
 * @see Mercator projection formulas : https://en.wikipedia.org/wiki/Mercator_projection
 * 
 * @param lon Longitude du point à convertir
 * @param lat Latitude du point à convertir
 * @param window_width Largeur de la fenêtre d'affichage en pixels
 * @param window_height Hauteur de la fenêtre d'affichage en pixels
 * 
 * @return Les coordonnées graphiques (x, y) correspondantes
 */
int lon_to_x(float lon, int window_width) {
    float cos_factor = cos(LAT_CENTER * PI / -180.0f);
    return (int)((lon - LON_MIN) / (LON_MAX - LON_MIN) * window_width * cos_factor
                 + window_width * (1.0f - cos_factor) / 2.0f);
}

int lat_to_y(float lat, int window_height) {
    float cos_factor = cos(LAT_CENTER * PI / -180.0f);
    return (int)((LAT_MAX - lat) / (LAT_MAX - LAT_MIN) * window_height * cos_factor
                 + window_height * (1.0f - cos_factor) / 2.0f);
}
