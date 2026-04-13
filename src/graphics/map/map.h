#ifndef MAP_H
#define MAP_H

/* Coordonnées  nord sud ouest est */
#define LAT_MAX  51.1f
#define LAT_MIN  41.3f
#define LON_MIN  -5.2f
#define LON_MAX   9.6f

/**
 * @struct Graphic
 * @brief Structure représentant les propriétés graphiques d'une vue cartographique.
 * @details Contient les coordonnées du centre et le niveau de zoom pour le rendu de la carte.
 * 
 * @member lat_center La coordonnée de latitude du centre de la carte
 * @member lon_center La coordonnée de longitude du centre de la carte
 * @member zoom Le niveau de zoom de la vue cartographique
 */
typedef struct {
    float lat_center;
    float lon_center;
    float zoom;
} Graphic;

/** 
* @struct GraphicTownPoint
* @brief Structure représentant un point de ville à afficher sur la carte.
* @details Contient les coordonnées géographiques et les informations d'identification pour un marqueur de ville.

* @member lat Coordonnée de latitude de la ville
* @member lon Coordonnée de longitude de la ville
* @member name Nom de la ville (99 caractères max + terminateur nul)
**/
typedef struct {
    float lat;
    float lon;
    char name[100];
} GraphicTownPoint;

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
int lon_to_x(float lon, int window_width);
int lat_to_y(float lat, int window_height);

#endif
