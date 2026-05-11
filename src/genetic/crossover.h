#ifndef CROSSOVER_H
#define CROSSOVER_H

#include "generation.h"

/*
 * Effectue le crossover entre deux parents et produit un enfant.
 * parent1, parent2 : les deux individus parents
 * child            : l'enfant à remplir (doit être déjà alloué avec nb_hop)
 * nb_hop           : nombre d'hôpitaux par individu
 * Retourne 0 si succès, 1 si erreur.
 */
int crossover(const Individu *parent1, const Individu *parent2,
              Individu *child, int nb_hop);

/*
 * Applique le crossover sur toute une génération par paires.
 * Produit une nouvelle génération d'enfants de même taille.
 * Retourne 0 si succès, 1 si erreur.
 */
int crossover_generation(const Generation *gen, Generation *children, int nb_hop);

/* Version qui retourne la génération — nouvelle */
Generation crossover_generation_new(const Generation *gen, int nb_hop);

#endif /* CROSSOVER_H */