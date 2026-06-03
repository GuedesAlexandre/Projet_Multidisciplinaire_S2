#ifndef CSV_EXPORT_H
#define CSV_EXPORT_H

#include "../data/municipality.h"
#include "../genetic/generation.h"

/*
 * Exporte la meilleure solution dans un fichier CSV.
 *
 * Colonnes :
 *   insee_code        - code INSEE de la commune
 *   a_hopital         - 1 si la commune a un hopital dans la solution, 0 sinon
 *   est_chru          - 1 si hopital ET population >= CHRU_POP_THRESHOLD
 *   hopital_desservant - code INSEE de l'hopital le plus proche dans le rayon
 *                        (vide si aucun hopital ne couvre la commune)
 *
 * Retourne 0 en cas de succes, 1 en cas d'erreur.
 */
int csv_export_best_solution(const char *output_path,
                             const MunicipalityTable *table,
                             const Individu *best);

#endif /* CSV_EXPORT_H */
