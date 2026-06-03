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

Lors de la première génération, le paramètre `nb_hop` de la configuration (par défaut, **2000**) n'est plus une contrainte stricte mais une **limite maximale**.

La taille initiale de chaque individu est tirée aléatoirement via `1 + rand() % nb_hop`, garantissant une population de départ diversifiée en termes de nombre d'hôpitaux. Les communes sont ensuite choisies de façon **uniforme** parmi toutes les communes disponibles (la méthode pondérée par population a été supprimée).

> La fonction `generation_init_weighted()` et ses helpers (`weighted_pick`, `pick_unique_weighted`) ont été retirés : l'initialisation passe désormais exclusivement par `generation_init_random()`.

## Sélection

La sélection s'effectue par **tournoi** (ex: `tournament_k = 5`) : on crée un ring de 5 individus choisis au hasard, et seul celui ayant la meilleure fitness obtient le droit de se reproduire.
Une fine marge aléatoire (`selection_random_rate = 0.10`) contourne par instant cette étape pour conserver de la richesse génétique. Le mécanisme d'élitisme préserve quant à lui les `elite_count` absolus de chaque génération sans modification.

## Croisement à taille variable (Crossover Hybride)

Deux parents croisés génèrent un enfant de taille inédite. La fonction `crossover()` n'accepte plus de paramètre `nb_hop` fixe : la taille de l'enfant est **calculée à partir des parents** :

1. **Évaluation de taille** : La longueur de l'enfant est un tirage aléatoire compris entre `min(parent1->nb_hop, parent2->nb_hop)` et `max(parent1->nb_hop, parent2->nb_hop)`.
2. **Héritage** : L'enfant pioche indépendamment, index par index, la ville provenant du patrimoine du Père ou de la Mère avec 50% de probabilité. Si un parent est plus court que l'index courant, on pioche aléatoirement dans ses gènes existants.
3. **Résolution des conflits** : Si le gène est déjà présent (pour éviter 2 hôpitaux dans la même commune), on cherche de façon déterministe le premier gène non utilisé en parcourant les réserves de chaque parent.

## Mutations Spatiales et Structurelles

La mutation ne modifie plus que la composition spatiale (quelle ville ?), elle modifie désormais **la longueur du génome**. Lorsqu'une mutation survient, elle a statistiquement :
    - **45% de chances de Remplacer** un hôpital par une autre commune non présente (Exploration Spatiale).
    - **35% de chances de Supprimer** un hôpital aléatoirement de l'individu (`realloc` avec réduction de taille). Permet l'optimisation des coûts (Élagage).
    - **20% de chances d'Ajouter** un nouvel hôpital aléatoire à l'individu (`realloc` avec augmentation de taille). Permet l'augmentation de la couverture (Colonisation).

## Mutation Guidée (Gain-Gard)

La mutation guidée (`guided_mutation_rate = 0.40`) a été entièrement repensée. Au lieu de remplacer l'hôpital le moins couvrant par une commune désertique aléatoire pondérée, elle agit désormais en **deux phases successives** sur chaque individu sélectionné :

### Phase 1 — Ajouts gain-gardés
Pour chaque individu, on calcule d'abord un vecteur `cover_count[c]` comptant combien d'hôpitaux de l'individu couvrent la commune `c`. On tente ensuite d'ajouter un hôpital **uniquement si** son gain (population de communes désertiques qu'il desservirait) dépasse la pénalité `FITNESS_PENALTY_HOP` (5000). Si l'ajout ne s'amortit pas, on n'ajoute rien. On répète jusqu'à `GUIDED_MAX_ROUNDS` tentatives ou saturation.

### Phase 2 — Élagage
On parcourt tous les hôpitaux de l'individu et on retire ceux dont la **couverture unique** (communes qu'eux seuls couvrent) représente une population inférieure à `FITNESS_PENALTY_HOP`. Retirer un tel hôpital fait gagner de la fitness car on économise la pénalité sans sacrifier de couverture irremplaçable.

## Immigrants et Redémarrages (Restarts)

- **Immigrants** : À chaque génération, certains individus de la fin du tableau sont écrasés par de nouveaux "immigrants" totalement aléatoires (taille variable). Le pourcentage de remplacement (`immigrants_rate`) augmente dynamiquement face à la stagnation (jusqu'à un plafond de **`0.30`**).
- **Restart** : Si aucune nouvelle « meilleure fitness all-time » n'a été trouvée depuis `restart_threshold` générations, une extinction massive a lieu. La population est relancée au hasard complète, en ne conservant que l'hyper-élite de la run précédente. La méthode `generation_init_weighted()` n'est plus utilisée lors du reseed.

## Export de la meilleure solution

À l'issue de la boucle principale, la meilleure solution trouvée est exportée automatiquement dans le fichier `best_solution.csv` via `csv_export_best_solution()`. Le fichier contient pour chaque commune :

| Colonne              | Description                                                                 |
|----------------------|-----------------------------------------------------------------------------|
| `insee_code`         | Code INSEE de la commune                                                    |
| `a_hopital`          | `1` si la commune accueille un hôpital dans la solution, `0` sinon          |
| `est_chru`           | `1` si la commune a un hôpital **et** une population ≥ seuil CHRU           |
| `hopital_desservant` | Code INSEE de l'hôpital le plus proche dans le rayon (vide si non couvert)  |

## La Structure `GAConfig`

Cette structure paramétrique regroupe le jeu de clés dirigeant le réseau d'évolution :

```c
typedef struct {
    int population_size;         /* Ex: 800 */
    int nb_hop;                  /* Borne MAXIMALE de la taille initiale des individus (Ex: 2000) */
    int max_generations;         /* Ex: 1000 */

    int elite_count;             /* Individus sanctuarisés */
    int tournament_k;            /* Taille de l'arène de sélection (Ex: 5) */

    double selection_random_rate;  /* Ex: 0.10 */
    double mutation_rate_start;    /* Ex: 0.30 */
    double mutation_rate_end;      /* Ex: 0.05 */

    double immigrants_rate;
    double immigrants_rate_max;    /* Ex: 0.30 */

    double guided_mutation_rate;   /* Taux de mutation guidée (Ex: 0.40) */

    int restart_threshold;       /* Nb de générations sans progrès avant extinction */
    int max_restarts;
    int stagnation_limit;        /* Arrêt complet au bout de X générations d'inactivité */

    double target_fitness;       /* Borne de victoire mathématique absolue (Ex: 55 000 000) */
    int verbose;
} GAConfig;
```

### Valeurs par défaut par rapport à notre dernière version

| Paramètre              | Ancienne valeur | Nouvelle valeur |
|------------------------|-----------------|-----------------|
| `nb_hop`               | 1120            | **2000**        |
| `tournament_k`         | 3               | **5**           |
| `mutation_rate_start`  | 0.20            | **0.30**        |
| `immigrants_rate_max`  | 0.40            | **0.30**        |
| `selection_random_rate`| 0.05            | **0.10**        |
| `guided_mutation_rate` | 0.03            | **0.40**        |
| `target_fitness`       | 50 000 000      | **55 000 000**  |
