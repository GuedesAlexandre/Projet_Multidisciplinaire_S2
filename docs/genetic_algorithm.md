# Definition de notre algorithme genetique

## Vue d'ensemble (implementation actuelle)

Notre algorithme genetique manipule une population d'individus. Chaque individu encode un ensemble fixe de communes choisies comme hopitaux (nombre fixe `nb_hop`). La population evolue avec elitisme, selection par tournoi, crossover uniforme, mutation, et injection d'immigrants. Un mecanisme de restart se declenche en cas de stagnation.

La fitness a maximiser est :

$$
fitness = 65141355 - habitants\_desert - 5000 \times nb\_hop + 4000 \times nb\_chru
$$

Ou :
- `habitants_desert` = population non couverte par un hopital a moins de 10 km.
- `nb_chru` = nombre d'hopitaux places dans une commune avec population >= 80 000.

## Donnees et representation

### Structure des communes
Le CSV est parse dans une table dynamique `MunicipalityTable` contenant des `Municipality` (code INSEE, nom, departement, population, latitude, longitude). Le tableau est charge une seule fois au demarrage et ne change plus.

### Individu
Un individu stocke uniquement des indices vers la table des communes, plus ses metadonnees :

```c
typedef struct {
    int   *indices;  /* indices de communes choisies */
    int    nb_hop;   /* nombre fixe d'hopitaux */
    double fitness;
    int    nb_chru;
} Individu;
```

Le nombre d'hopitaux par individu est fixe (parametre `nb_hop` du GA). La penalite sur `nb_hop` reste dans la fitness, mais `nb_hop` ne varie pas au cours de l'evolution.

## Acceleration du calcul de couverture

Pour eviter de recalculer toutes les distances a chaque evaluation, on precalcule un index de couverture :

- `CoverageIndex.offsets[i]` et `offsets[i+1]` donnent le segment dans `indices[]` des communes couvertes par un hopital place a la commune `i`.
- `CoverageIndex.indices` contient toutes les listes concatenees.

L'evaluation d'un individu marque les communes couvertes a partir de ces listes, puis additionne la population non marquee.

## Initialisation

La population initiale est generee par tirage pondéré : les communes les plus peuplees ont plus de chance d'etre choisies comme hopitaux. On garantit l'absence de doublons dans les indices.

Parametres par defaut (config):
- `population_size = 800`
- `nb_hop = 1120`
- `max_generations = 1000`
- `elite_count = 2`
- `tournament_k = 3`
- `mutation_rate_start = 0.20`, `mutation_rate_end = 0.05`
- `immigrants_rate = 0.10`, `immigrants_rate_max = 0.40`
- `restart_threshold = 30`, `max_restarts = 5`
- `target_fitness = 50 000 000`, `stagnation_limit = 120`

## Selection

La selection se fait par tournoi (k = 3). Une part de selection aleatoire est possible (`selection_random_rate`) pour garder de la diversite. L'elitisme copie les `elite_count` meilleurs individus directement dans la generation suivante.

## Croisement (uniforme hybride)

Pour chaque gene, on choisit aleatoirement le gene du parent 1 ou 2. En cas de doublon, on cherche deterministiquement le premier gene non utilise (d'abord dans parent 2, puis parent 1). Cela garantit un enfant sans doublons.

## Mutation

La mutation applique au hasard :
- remplacement d'un hopital par une autre commune non presente, ou
- permutation de deux positions (swap).

Le taux de mutation evolue lineairement de `mutation_rate_start` vers `mutation_rate_end`, avec un boost en cas de stagnation (jusqu'a un plafond fixe a 0.35).

## Immigrants

Apres mutation, une partie des individus est remplacee par des individus aleatoires (immigrants). Le taux augmente quand la stagnation persiste, dans la limite de `immigrants_rate_max`.

## Restarts et criteres d'arret

Si la stagnation depasse `restart_threshold`, on reinitialise une population (tirage pondere) en conservant les elites. L'algorithme s'arrete quand :
- la fitness cible est atteinte,
- le nombre max de generations est atteint,
- ou la stagnation atteint `stagnation_limit`.

## Parcours d'execution

```
1) Charger le CSV -> table des communes
2) Construire l'index de couverture (CoverageIndex)
3) Initialiser la population (tirage pondere)
4) Evaluer la population
5) Boucle de generations :
   - ajuster mutation/immigrants selon la stagnation
   - elitisme
   - selection par tournoi + crossover uniforme
   - mutation
   - immigrants
   - evaluation
   - test arret / restart
6) Afficher la meilleure fitness finale

```
## La GACONFIG KESAKO ?

`GAConfig` est la structure de configuration centrale de l'algorithme genetique. Elle regroupe tous les parametres ajustables en un seul endroit pour eviter les constantes dispersees dans le code.

Elle sert a :
- definir le comportement global (taille population, generations, etc.),
- piloter l'exploration/exploitation (mutation, selection, immigrants),
- encadrer la robustesse (stagnation, restart, arret).

Exemple de champs typiques :

```c
typedef struct {
    int population_size;
    int nb_hop;
    int max_generations;

    int elite_count;
    int tournament_k;
    double selection_random_rate;

    double mutation_rate_start;
    double mutation_rate_end;
    double mutation_rate_max; /* ex: 0.35 */

    double immigrants_rate;
    double immigrants_rate_max;

    int restart_threshold;
    int max_restarts;
    int stagnation_limit;

    double target_fitness;
} GAConfig;
```

En pratique, une instance de `GAConfig` est chargee au demarrage (valeurs par defaut ou fichier), puis transmise aux fonctions d'initialisation, d'evaluation et d'evolution pour garder un comportement coherent et reproductible.

