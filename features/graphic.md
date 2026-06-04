# Module Graphique - Explications

## Conception du module graphique

Le module graphique est implémenté dans `graphic.c` et expose son API via `graphic.h`. Il repose sur la bibliothèque **MLV** pour la création de fenêtre et le rendu 2D. Il est découpé en deux sous-parties :

- `map/map.c` + `map/map.h` : projection géographique (longitude/latitude pour les pixels).
- `graphic.c` + `graphic.h` : dessin des communes, hôpitaux, statistiques et orchestration de l'affichage par génération.

## Projection géographique — `map/map.c`

La France métropolitaine est délimitée par des bornes géographiques fixes :

```
LAT_MAX = 51.1,  LAT_MIN = 41.3
LON_MIN = -5.2,  LON_MAX =  9.6
```

Les deux fonctions `lon_to_x` et `lat_to_y` convertissent des coordonnées GPS en pixel pour l'affichage à l'écran. La formule de base est une règle de trois :

```
x = (lon - LON_MIN) / (LON_MAX - LON_MIN) * window_width
y = (LAT_MAX - lat) / (LAT_MAX - LAT_MIN) * window_height
```

***Un facteur de correction `cos(LAT_CENTER * PI / 180)` est appliqué sur les deux axes. Sans ce facteur, la carte serait étirée horizontalement car un degré de longitude représente une distance réelle plus courte que un degré de latitude aux latitudes de la France (~46°). Ce `cos_factor` compense cette distorsion en réduisant légèrement la largeur de la carte et en la centrant. C'est une approximation simplifiée de la projection de Mercator.***

## Stratégie de double buffer

L'affichage utilise une stratégie de **double buffer** pour éviter de recalculer le fond de carte (35 000 points) à chaque génération :

1. Au démarrage, `draw_map_of_towns` dessine une fois toutes les communes (points bleus) sur une `MLV_Image *bg` (image en mémoire, pas encore visible à l'écran).
2. À chaque génération, `display_generation` commence par `MLV_draw_image(bg, 0, 0)` qui recopie directement cette image sur la fenêtre, puis dessine les hôpitaux par-dessus.

***Sans ce mécanisme, il faudrait redessiner les 35 000 points à chaque rendu, ce qui est bien plus lent. L'image `bg` est une optimisation qui joue le rôle de cache graphique statique.***

## Fonctions de dessin

### `draw_map_of_towns`

Dessine toutes les communes sous forme de **points bleus** sur l'image de fond `background`. Utilise `MLV_draw_point_on_image` (dessin sur image, pas sur la fenêtre) pour construire le cache statique.

### `draw_chru`

Dessine un **cercle vert de rayon 4** pour les communes choisies comme hôpital et dont la population dépasse `CHRU_POP_THRESHOLD` (80 000 habitants). Ce seuil correspond à la définition d'un Centre Hospitalier Régional Universitaire.

### `draw_hospitals_from_individu`

Parcourt les `nb_hop` hôpitaux d'un individu et pour chaque hôpital :
- Si la commune dépasse `CHRU_POP_THRESHOLD`, on appel à `draw_chru`.
- Sinon c'est juste des cercles rouges de rayon 1 (hôpital standard)

***Ce dessin se fait directement sur la fenêtre, ce qui permet de l'effacer à la génération suivante en réappliquant le fond avec `MLV_draw_image(bg, 0, 0)`.***

### `generation_get_best_index`

Parcourt linéaire de la génération pour trouver l'indice de l'individu avec la meilleure fitness. Seul cet individu est affiché.

## Affichage par génération — `display_generation`

C'est la fonction centrale du module. Elle prend en entrée :
- `bg` : l'image de fond pré-calculée
- `table` : la table des communes (pour les statistiques)
- `gen` : la génération courante évaluée
- `gen_index` : le numéro de génération (affiché à l'écran)
- `avg_fitness` : la fitness moyenne de la génération
- `total_population` : population totale (pour calculer les pourcentages)

Elle affiche à droite de la fenêtre (de `MAP_WIDTH + 10` à `WINDOW_WIDTH`) les statistiques du meilleur individu :
- Numéro de génération (jaune)
- Nombre d'hôpitaux et de C.H.R.U. (blanc / cyan)
- Habitants et communes éloignés, avec leurs pourcentages (blanc)
- Fitness (vert)
- Taille de la population et fitness moyenne (blanc)

***`MLV_draw_text` interprète son argument texte comme une chaîne de format `printf`. Le caractère `%` doit être doublé (`%%`) dans le buffer passé à MLV pour s'afficher correctement. Comme on utilise d'abord `sprintf` pour construire le buffer (qui transforme `%%` en `%`), il faut écrire `%%%%` dans le format de `sprintf` pour obtenir `%%` dans le buffer, que MLV affiche finalement comme `%`.***
+
## Throttling de l'affichage

Sans optimisation, `display_generation` serait appelée à chaque génération (jusqu'à 1000 fois), ce qui rallonge l'exécution de plusieurs minutes.

Le mécanisme de throttling dans `main.c` limite les rafraîchissements à **au plus 1 toutes les `DISPLAY_INTERVAL_MS` millisecondes** (100ms par défaut) :

```c
unsigned long now = MLV_get_time();
if (now - last_display >= DISPLAY_INTERVAL_MS) {
    display_generation(...);
    last_display = now;
}
```

***`MLV_get_time()` retourne un `unsigned long` représentant les millisecondes depuis l'ouverture de la fenêtre. Le type `unsigned long` est la soustraction `now - last_display` qui est toujours correcte en non signée, même en cas de dépassement de la valeur max(wrap-around).***

Les **restarts** de l'algorithme génétique forcent toujours un rendu immédiat, indépendamment du throttling, pour signaler visuellement le changement de population.

## Cycle de vie de la fenêtre

```
init_window()           --> MLV_create_window (1200×800)
MLV_create_image()      --> allocation du buffer de fond
draw_map_of_towns()     --> dessin du fond une seule fois
    [boucle GA]
    display_generation() --> rendu throttlé
    [fin boucle]
display_generation()    --> rendu final (meilleur résultat)
MLV_wait_mouse()        --> attente d'un clic pour fermer
MLV_free_image()        --> libération du buffer de fond
close_window()          --> MLV_free_window
```
