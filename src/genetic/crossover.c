#include "crossover.h"
#include <stdlib.h>

/*
 * Vérifie si la valeur v est présente dans arr[0..len-1].
 */
static int contains(const int *arr, int len, int v)
{
    int i;
    for (i = 0; i < len; i++)
        if (arr[i] == v) return 1;
    return 0;
}

/*
 * Crossover à point de coupure entre parent1 et parent2.
 * L'enfant hérite des k premiers gènes de parent1,
 * puis est complété avec les gènes de parent2 sans doublons.
 */
int crossover(const Individu *parent1, const Individu *parent2,
              Individu *child, int nb_hop)
{
    int i, j, count;
    int cut;

    if (!parent1 || !parent2 || !child) return 1;

    child->indices = (int *)malloc(sizeof(int) * nb_hop);
    if (!child->indices) return 1;

    child->nb_hop = nb_hop;
    child->fitness = 0.0;
    child->nb_chru = 0;

    /* Point de coupure aléatoire entre 1 et nb_hop-1 */
    cut = 1 + rand() % (nb_hop - 1);

    /* Copie les 'cut' premiers gènes du parent1 */
    for (i = 0; i < cut; i++)
        child->indices[i] = parent1->indices[i];

    count = cut;

    /* Complète avec les gènes de parent2 non déjà présents */
    for (j = 0; j < nb_hop && count < nb_hop; j++) {
        if (!contains(child->indices, count, parent2->indices[j])) {
            child->indices[count] = parent2->indices[j];
            count++;
        }
    }

    return 0;
}

/*
 * Applique le crossover sur toute la génération par paires successives.
 * (0,1) -> enfant 0, (2,3) -> enfant 1, etc.
 * Si la taille est impaire, le dernier individu est copié tel quel.
 */
int crossover_generation(const Generation *gen, Generation *children, int nb_hop)
{
    int i;

    if (!gen || !children) return 1;

    children->size = gen->size;
    children->capacity = gen->size;
    children->best_index = -1;
    children->items = (Individu *)malloc(sizeof(Individu) * gen->size);
    if (!children->items) return 1;

    for (i = 0; i + 1 < gen->size; i += 2) {
        if (crossover(&gen->items[i], &gen->items[i + 1],
                      &children->items[i], nb_hop) != 0) return 1;
        if (crossover(&gen->items[i + 1], &gen->items[i],
                      &children->items[i + 1], nb_hop) != 0) return 1;
    }

    /* Si taille impaire : copie du dernier individu */
    if (gen->size % 2 != 0) {
        int last = gen->size - 1;
        children->items[last].nb_hop  = nb_hop;
        children->items[last].fitness = 0.0;
        children->items[last].nb_chru = 0;
        children->items[last].indices = (int *)malloc(sizeof(int) * nb_hop);
        if (!children->items[last].indices) return 1;
        for (i = 0; i < nb_hop; i++)
            children->items[last].indices[i] = gen->items[last].indices[i];
    }

    return 0;
}

Generation crossover_generation_new(const Generation *gen, int nb_hop)
{
    int i, p1, p2;
    Generation children;

    /* Initialisation par défaut en cas d'erreur */
    children.items = NULL;
    children.size = 0;
    children.capacity = 0;
    children.best_index = -1;

    if (!gen || nb_hop <= 0) return children;

    children.items = (Individu *)malloc(sizeof(Individu) * gen->size * 2);
    if (!children.items) return children;

    children.size = gen->size * 2;
    children.capacity = gen->size * 2;

    for (i = 0; i < gen->size * 2; i++) {
        /* Deux parents différents tirés au hasard */
        p1 = rand() % gen->size;
        do { p2 = rand() % gen->size; } while (p2 == p1);

        if (crossover(&gen->items[p1], &gen->items[p2],
                      &children.items[i], nb_hop) != 0) {
            /* En cas d'erreur on libère ce qui a été alloué et on retourne vide */
            children.size = i;
            generation_free(&children);
            children.size = 0;
            children.capacity = 0;
            children.best_index = -1;
            return children;
        }
    }

    return children;
}