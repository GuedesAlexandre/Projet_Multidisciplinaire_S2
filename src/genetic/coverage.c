#include "coverage.h"
#include "fitness.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

/*
Cette fonction calcule pour chaque commune la liste des communes couvertes par un hopital situé dans cette commune.
L'index de couverture est stocké dans une structure CoverageIndex, qui contient :
- count : le nombre de communes
- offsets : un tableau d'entiers de taille count + 1, où offsets[i] donne l'index de départ dans indices[] pour la commune i, et offsets[i+1] donne l'index de fin (exclusif) pour la commune i. Ainsi, les communes couvertes par la commune i sont dans indices[offsets[i]..offsets[i+1]-1].
- indices : un tableau d'entiers qui contient les listes de communes couvertes, concaténées les unes après les autres. La taille de ce tableau est offsets[count], qui correspond au nombre total de couvertures (somme des counts[i]).
*/
/*
Comment cette algorithme fonctionne-t-il ?
1. On commence par calculer pour chaque commune i le nombre de communes qu'elle couvre, en utilisant la distance de Haversine pour vérifier si les communes j sont à moins de HOSPITAL_RADIUS_KM de la commune i.
 On stocke ces comptes dans le tableau counts[].
2. Ensuite, on construit le tableau offsets[] en cumulant les counts[i]. offsets[0] est initialisé à 0, et offsets[i+1] = offsets[i] + counts[i]. Cela nous donne les indices de départ et de fin pour chaque commune i dans le tableau indices[].
3. On alloue le tableau indices[] avec la taille totale nécessaire (offsets[count]).
4. Enfin, on remplit le tableau indices[] en parcourant à nouveau les communes i et j, et en ajoutant j à la liste de couverture de i (dans indices[]) 
si j est à moins de HOSPITAL_RADIUS_KM de i. On utilise un tableau pos[] pour suivre la position actuelle d'insertion dans indices[] pour chaque commune i,
 en initialisant pos[i] à offsets[i] et en l'incrémentant à chaque insertion.
5. On stocke le nombre total de communes dans out_index->count et on retourne 0 pour indiquer le succès. En cas d'erreur (paramètres invalides ou échec d'allocation mémoire), on retourne 1.
*/
int coverage_build(const MunicipalityTable *table, CoverageIndex *out_index)
{
    int i, j;
    int count;
    int *counts;
    int *pos;
    double lat_diff_limit;
    const Municipality *items;

    if (!table || !out_index)
        return 1;
    if (table->count == 0)
        return 1;

    count = table->count;
    items = table->items;

    counts = (int *)calloc(count, sizeof(int));
    if (!counts)
        return 1;

    /* Pourquoi on divise par 111 ? Parce que 1 degré de latitude correspond à environ 111 km,
    donc on convertit la distance de couverture en degrés de latitude pour limiter les comparaisons
    aux communes qui sont à une distance raisonnable en latitude, ce qui améliore les performances.*/
    lat_diff_limit = (double)HOSPITAL_RADIUS_KM / 111.0; /* Je suis obligé de caster en double pour éviter un overflow dans le calcul du modulo */

    for (i = 0; i < count; i++)
    {
        double lat_i = items[i].latitude;
        double lon_i = items[i].longitude;
        for (j = 0; j < count; j++)
        {
            double lat_j = items[j].latitude;
            if (fabs(lat_i - lat_j) > lat_diff_limit)
                continue;
            if (haversine_km(lat_i, lon_i, lat_j, items[j].longitude) < (double)HOSPITAL_RADIUS_KM)
            {
                counts[i]++;
            }
        }
    }

    out_index->offsets = (int *)malloc(sizeof(int) * (count + 1));
    if (!out_index->offsets)
    {
        free(counts);
        return 1;
    }

    out_index->offsets[0] = 0;
    for (i = 0; i < count; i++)
        out_index->offsets[i + 1] = out_index->offsets[i] + counts[i];

    out_index->indices = (int *)malloc(sizeof(int) * out_index->offsets[count]);
    if (!out_index->indices)
    {
        free(out_index->offsets);
        out_index->offsets = NULL;
        free(counts);
        return 1;
    }

    pos = (int *)malloc(sizeof(int) * count);
    if (!pos)
    {
        free(out_index->indices);
        free(out_index->offsets);
        out_index->indices = NULL;
        out_index->offsets = NULL;
        free(counts);
        return 1;
    }

    for (i = 0; i < count; i++)
        pos[i] = out_index->offsets[i];

    for (i = 0; i < count; i++)
    {
        double lat_i = items[i].latitude;
        double lon_i = items[i].longitude;
        for (j = 0; j < count; j++)
        {
            double lat_j = items[j].latitude;
            if (fabs(lat_i - lat_j) > lat_diff_limit)
                continue;
            if (haversine_km(lat_i, lon_i, lat_j, items[j].longitude) <  HOSPITAL_RADIUS_KM)
            {
                out_index->indices[pos[i]++] = j;
            }
        }
    }

    out_index->count = count;

    free(pos);
    free(counts);
    return 0;
}

void coverage_free(CoverageIndex *index)
{
    if (!index)
        return;
    if (index->offsets)
        free(index->offsets);
    if (index->indices)
        free(index->indices);
    index->offsets = NULL;
    index->indices = NULL;
    index->count = 0;
}
