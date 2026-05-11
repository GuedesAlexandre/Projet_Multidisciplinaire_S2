#include "selection.h"
#include <stdlib.h>

/*
 * Comparateur pour qsort : tri par fitness décroissante.
 */
static int compare_fitness(const void *a, const void *b)
{
    const Individu *ia = (const Individu *)a;
    const Individu *ib = (const Individu *)b;

    if (ib->fitness > ia->fitness) return 1;
    if (ib->fitness < ia->fitness) return -1;
    return 0;
}

/*
 * Sélectionne les nb_keep meilleurs individus.
 * Les individus éliminés sont libérés de la mémoire.
 */
void selection(Generation *gen, int nb_keep)
{
    int i;

    if (!gen || nb_keep <= 0 || nb_keep >= gen->size) return;

    /* Tri par fitness décroissante */
    qsort(gen->items, gen->size, sizeof(Individu), compare_fitness);

    /* Libère la mémoire des individus éliminés */
    for (i = nb_keep; i < gen->size; i++) {
        free(gen->items[i].indices);
        gen->items[i].indices = NULL;
    }

    /* Met à jour la taille */
    gen->size = nb_keep;
    gen->best_index = 0; /* le meilleur est toujours en position 0 après le tri */
}