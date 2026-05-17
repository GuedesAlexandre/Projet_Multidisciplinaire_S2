#ifndef MUTATION_H
#define MUTATION_H

#include "generation.h"

#define MUTATION_RATE 0.05

/*
 * Applique une mutation aleatoire sur un individu (deplacement ou ajout ou suppression)
 * commune_count : nombre total de communes disponibles.
 * Retourne 0 si success et 1 en si erreur memoire.
 */
int mutation_apply(Individu* ind, int commune_count);

/*
 * Applique mutation_apply sur chaque individu de la generation avec une probabilite MUTATION_RATE.
 * Retourne 0 si succes et 1 si erreur memoire.
 */
int mutation_generation(Generation* gen, int commune_count);

#endif /* MUTATION_H */
