# Définition de notre algorithme génétique

## Vue d'ensemble (implémentation actuelle)

Notre algorithme génétique manipule une population d'individus. Contrairement à une approche classique, chaque individu encode un ensemble **variable** de communes choisies comme hôpitaux. La longueur du génome (`nb_hop`) est propre à chaque individu et évolue au fil des générations.

L'objectif est de trouver le point d'équilibre parfait entre une couverture universelle de la population et une minimisation des coûts (frais de construction/gestion représentés par le nombre d'hôpitaux utilisés).

La fonction de fitness (score de survie) à maximiser est :

$$
fitness = FITNESS\_BASE\_POPULATION - habitants\_desert - (5000 \times nb\_hop) + (4000 \times nb\_chru)
$$

Où :
    - `FITNESS_BASE_POPULATION` = Constante représentant la population française (~ 65 141 355).
    - `habitants_desert` = Population non couverte par un hôpital à moins de 10 km.
    - `5000` = Pénalité stricte appliquée pour **chaque** hôpital construit.
    - `nb_chru` = Nombre d'hôpitaux placés dans de grands bassins urbains (commune >= 80 000 habitants).

L'algorithme s'appuie sur le principe de pénalité : il va chercher à réduire son code génétique (fermer des hôpitaux inutiles) si le gain en population couverte n'amortit pas le coût fictif de 5000 points, limitant ainsi la prolifération excessive des centres médicaux.

## Données et représentation

### Structure des communes
Le CSV est parsé dans une table dynamique `MunicipalityTable` contenant des objets `Municipality` (code INSEE, nom, département, population, latitude, longitude). Ce tableau est évalué et chargé une seule fois au démarrage.

### Individu
Un individu stocke uniquement des indices vers la table des communes, ainsi que ses métadonnées. L'allocation de son tableau d'indices `indices[]` est rendue dynamique grâce à `malloc` et `realloc` pour suivre les évolutions.

    ```c
    typedef struct {
        int   *indices;          /* tableau dynamique des indices de communes */
        int    nb_hop;           /* nombre variable d'hôpitaux de cet individu */
        double fitness;          /* score de l'individu */
        int    nb_chru;          /* métrique de suivi des gros CHRU */
        long   habitants_desert; /* métrique de suivi pour le rapport */
        long   communes_desert;
    } Individu;
    ```

## Initialisation

Lors de la première génération, le paramètre `nb_hop` de la configuration (par défaut, 1460) n'est plus une contrainte stricte mais une **limite maximale**. 
La taille initiale des individus est déterminée aléatoirement (soit `1 + rand() % max_nb_hop`). Les villes hôpitaux de l'individu sont ensuite tirées au hasard avec une méthode "pondérée" (les communes les plus peuplées ont plus de chance d'être piochées initialement).

## Sélection

La sélection s'effectue par **tournoi** (ex: `tournament_k = 3`) : on crée un ring de 3 individus choisis au hasard, et seul celui ayant la meilleure fitness obtient le droit de se reproduire. 
Une fine marge aléatoire (`selection_random_rate`) contourne par instant cette étape pour conserver de la richesse génétique. Le mécanisme d'élitisme préserve quant à lui les `elite_count` absolus de chaque génération sans modification.

## Croisement à taille variable (Crossover Hybride)

Deux parents croisés génèrent un enfant de taille inédite :
    1. **Évaluation de taille** : La longueur de l'enfant est un tirage aléatoire englobé entre la longueur de son *Parent 1* et celle de son *Parent 2*.
    2. **Héritage** : L'enfant pioche indépendamment, index par index, la ville provenant du patrimoine du Père ou de la Mère avec 50% de probabilité.
    3. **Résolution des conflits** : Si le gène est déjà présent (pour éviter 2 hôpitaux dans la même commune), on cherche de façon déterministe le premier gène non utilisé en parcourant les réserves de chaque parent.

## Mutations Spatiales et Structurelles

La mutation ne modifie plus que la composition spatiale (quelle ville ?), elle modifie désormais **la longueur du génome**. Lorsqu'une mutation survient, elle a statistiquement :
    - **45% de chances de Remplacer** un hôpital par une autre commune non présente (Exploration Spatiale).
    - **35% de chances de Supprimer** un hôpital aléatoirement de l'individu (`realloc` avec réduction de taille). Permet l'optimisation des coûts (Élagage).
    - **20% de chances d'Ajouter** un nouvel hôpital aléatoire à l'individu (`realloc` avec augmentation de taille). Permet l'augmentation de la couverture (Colonisation).

## Immigrants et Redémarrages (Restarts)

- **Immigrants** : À chaque génération, certains individus de la fin du tableau sont écrasés par de nouveaux "immigrants" totalement aléatoires (taille variable). Le pourcentage de remplacement (`immigrants_rate`) augmente dynamiquement face à la stagnation (jusqu'à un plafond de `0.40`).
- **Restart** : Si aucune nouvelle « meilleure fitness all-time » n'a été trouvée depuis `restart_threshold` générations, une extinction massive a lieu. La population est relancée au hasard complète, en ne conservant que l'hyper-élite de la run précédente.

    ## La Structure `GAConfig`

    Cette structure paramétrique regroupe le jeu de clés dirigeant le réseau d'évolution :

    ```c
    typedef struct {
        int population_size;         /* Ex: 800 */
        int nb_hop;                  /* Borne MAXIMALE originelle de la taille d'individus */
        int max_generations;         /* Ex: 1000 */

        int elite_count;             /* Individus sanctuarisés */
        int tournament_k;            /* Taille de l'arène de sélection */
    
        double selection_random_rate;
        double mutation_rate_start;  
        double mutation_rate_end;    

        double immigrants_rate;
        double immigrants_rate_max;

        int restart_threshold;       /* Nombre fixe avant extinction */
        int max_restarts;
        int stagnation_limit;        /* Arrêt complet au bout de X d'inactivité */

        double target_fitness;       /* Borne de victoire mathématique absolue */
        int verbose;
    } GAConfig;
    ```


