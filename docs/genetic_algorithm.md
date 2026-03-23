# Définition de notre algorithme

## Comment ça marche théoriquement ?

Un algorithme génétique repose sur deux notions fondamentales : la population et l'individu. Un individu est une représentation encodée d'une solution candidate au problème. Une population est un ensemble d'individus évoluant ensemble au fil des générations.

L'algorithme se déroule de la façon suivante :
Initialisation. On génère une population initiale d'individus de façon aléatoire. Chaque individu représente une implantation possible d'hôpitaux sur le territoire.

Évaluation. On calcule la fitness de chaque individu. Dans notre projet, la fonction de fitness à maximiser est :

fi𝑡𝑛𝑒𝑠𝑠(𝑖𝑚𝑝𝑙𝑎𝑛𝑡𝑎𝑡𝑖𝑜𝑛) = 65141355 − ℎ𝑎𝑏𝑖𝑡𝑎𝑛𝑡_𝑑𝑒𝑠𝑒𝑟𝑡 − 5000 × 𝑛𝑏_ℎ𝑜𝑝 + 4000 × 𝑛𝑏_𝑐ℎ𝑟𝑢

Plus la fitness est élevée, meilleure est la solution : peu d'habitants en désert médical, peu d'hôpitaux (pour limiter les coûts) et un maximum de C.H.R.U.

Sélection. On sélectionne les individus les plus performants — typiquement la moitié supérieure de la population — pour servir de parents à la génération suivante.

Croisement (crossover). On combine des paires de parents pour produire de nouveaux individus enfants. L'idée est que deux bonnes implantations partielles peuvent, une fois combinées, donner une implantation globalement meilleure.

Mutation. On applique des modifications aléatoires sur certains individus avec une faible probabilité : ajout, suppression ou déplacement d'un hôpital. Ce mécanisme permet d'explorer de nouvelles zones de l'espace des solutions et d'éviter que la population converge trop tôt vers un optimum local.

Nouvelle génération. Les individus enfants remplacent leurs parents et forment la nouvelle population. On itère ce processus jusqu'à satisfaire un critère d'arrêt : nombre maximal de générations atteint, ou fitness suffisamment élevée pour considérer la solution satisfaisante.

## Comment ça s'implémente chez nous ?

## Définition d'un individu

Dans notre projet, une solution candidate est une sélection de communes parmi les ~34 900 communes de France métropolitaine hors Corse dans lesquelles on implante un hôpital. Le nombre d'hôpitaux n'est pas fixé a priori : c'est la fonction de fitness qui régule naturellement ce nombre via la pénalité -5000 × nb_hop.

### La structure correspondant à notre csv est la suivante : 

```c
#define MAX_COMMUNES 35000

typedef struct {
    int    code_insee;
    char   nom[100];
    int    code_region;
    char   nom_region[60];
    int    code_dept;
    char   nom_dept[60];
    int    altitude;
    int    population;
    double latitude;
    double longitude;
} Commune;

Commune communes[MAX_COMMUNES];
int nb_communes;

```

Un individu représente une sélection de communes parmi les 34 438 où l'on place un hôpital. Concrètement, un individu est un tableau d'indices pointant vers des entrées du tableau global communes[].

```c
typedef struct {
    int    *indices;
    int    nb_hop;   
    double fitness;
} Individu;
```

Le tableau global communes[] est chargé une seule fois au démarrage depuis le CSV. Il contient les 34 438 communes avec toutes leurs données (code INSEE, nom, population, lat/lon...). Ce tableau ne change jamais pendant l'exécution.

Un individu ne stocke pas les communes elles-mêmes — il stocke uniquement des indices dans ce tableau. C'est léger, rapide, et facile à manipuler

### Calcul de la fitness

Pour calculer la fitness, on doit savoir combien d'habitants sont en désert médical, c'est-à-dire dans aucune commune à moins de 10 km d'un hôpital. Pour ça, pour chaque commune sans hôpital, ont doit vérifier si elle est à moins de 10 km d'au moins un hôpital de l'individu.

Donc on devra coder une fonction distance qui utiliser la formule d'haversine.

### Le parcours du coup c'est quoi ? 
```
1. Charger le CSV
   └── parser chaque ligne → remplir communes[34438]

2. Initialiser la population
   └── pour chaque individu :
         tirer aléatoirement K indices dans [0, 34437]
         → remplir individu.indices[], individu.nb_hop = K

3. Boucle principale (jusqu'au critère d'arrêt)
   │
   ├── Évaluation
   │     └── pour chaque individu :
   │           pour chaque commune C :
   │             pour chaque hôpital H :
   │               si distance(C, H) < 10 → couverte
   │           → calculer habitant_desert, nb_chru
   │           → calculer fitness
   │
   ├── Sélection
   │     └── trier la population par fitness décroissante
   │         garder la moitié supérieure
   │
   ├── Croisement
   │     └── combiner des paires de parents
   │         → produire des enfants
   │
   ├── Mutation
   │     └── modifier aléatoirement quelques individus
   │         (ajout / suppression / déplacement d'hôpital)
   │
   └── Nouvelle génération
         └── les enfants remplacent les parents

4. Afficher la meilleure solution

Annexe : Définir un hopital (une commune = un hopital)

communes[] contient 34 438 communes
                        ↓
          on en tire K au hasard (ex: K=3000)
                        ↓
     ces K communes deviennent les hôpitaux
     de cet individu → stockés dans indices[]
     
```
