# Rapport de développement

## Parseur CSV
### Structure des données

Pour représenter une commune, on a introduit la struct `Municipality` dans `municipality.h`.  
Elle ne contient que les champs pertinent pour le projet projet afin de limiter l'utilisation de la mémoire et de simplifier la logique métier :

- `insee_code` : identifiant unique de la commune utilisé comme identifiant.
- `name[64]` : nom de la commune, utilisé pour la génération du PDF final. Le buffer est dimensionné à 64 caractères car la commune la plus longue du jeu de données fait 43 caractères, ce qui laisse une marge de sécurité pour éviter les dépassements.
- `dept_code` : code du département, utilisé pour regrouper les communes par département.
- `dept_name[32]` : nom du département, utilisé pour la génération du PDF. Le buffer fait 32 caractères et la valeur maximale est de 20 caractères.
- `population` : population totale de la commune, utilisée pour la fonction de fitness et pour les seuils CHRU.
- `latitude` / `longitude` : coordonnées géographiques utilisées pour placer les points sur la carte.

```
typedef struct {
    int    insee_code;    
    char   name[64];      
    int    dept_code;     
    char   dept_name[32]; 
    int    population;   
    double latitude;      
    double longitude;    
} Municipality;
```

On a aussi défini une structure `MunicipalityTable` qui encapsule un tableau dynamique de `Municipality` :

- `items` : pointeur vers le tableau de communes alloué dynamiquement.
- `count` : nombre de communes chargées.
- `capacity` : capacité actuelle du tableau.

```
typedef struct {
    Municipality *items;
    size_t   count;
    size_t   capacity;
} MunicipalityTable;
```

Une fonction  `municipality_table_free` gère la libération de la mémoire associée. Cela évite les fuites mémoire et rend l’API plus sûre à utiliser côté appelant, même en cas d’erreur d’allocation.

### Gestion de la mémoire et tableau dynamique

La table de communes est construite de manière dynamique au fur et à mesure de la lecture du fichier :

- Au début du parsing, `items` est `NULL`, `count` et `capacity` sont initialisés à 0.
- Avant d’ajouter une nouvelle commune, on vérifie si `count >= capacity`. Si oui, on appelle la fonction interne `table_grow`.
- `table_grow` applique une stratégie d’agrandissement exponentielle : si la capacité est nulle, elle commence à `INITIAL_CAP` (512), sinon elle double la capacité actuelle. Cette approche limite le nombre d’appels à `realloc` et offre de meilleures performances qu’une croissance linéaire.
- En cas d’échec de `realloc`, la fonction retourne une erreur mémoire, un message d’erreur est affiché, et on nettoie proprement la table via `municipality_table_free`.

Ce design sépare les responsabilités : le parseur gère l’allocation et la croissance du tableau, tandis que le reste du programme consomme simplement une `MunicipalityTable` déjà remplie.