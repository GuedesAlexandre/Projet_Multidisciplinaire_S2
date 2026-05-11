#ifndef SELECTION_H
#define SELECTION_H

#include "generation.h"

/*
 * Trie les individus de la génération par fitness décroissante
 * et ne garde que les 'nb_keep' meilleurs.
 * Modifie la génération sur place.
 */
void selection(Generation *gen, int nb_keep);

#endif /* SELECTION_H */