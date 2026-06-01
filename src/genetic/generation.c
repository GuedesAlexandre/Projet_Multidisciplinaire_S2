#include "generation.h"
#include <time.h>
#include "../tools/tools.h"

static int weighted_pick(const long *cumulative,
                         int count,
                         long total)
{
    long r;
    int lo, hi, mid;

    if (count <= 0) return -1;
    if (total <= 0) return rand() % count;

    r = (long)(((double)rand() / ((double)RAND_MAX + 1.0)) * (double)total);

    lo = 0;
    hi = count - 1;
    while (lo < hi) {
        mid = lo + (hi - lo) / 2;
        if (r < cumulative[mid]) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }
    return lo;
}

static int pick_unique_weighted(const long *cumulative,
                                int count,
                                long total,
                                const int *used,
                                int used_count)
{
    int tries = 0;
    int cand;

    while (1) {
        cand = weighted_pick(cumulative, count, total);
        if (cand < 0) return -1;
        if (!contains_int(used, used_count, cand)) return cand;
        tries++;
        if (tries > count * 2) {
            for (cand = 0; cand < count; cand++) {
                if (!contains_int(used, used_count, cand)) return cand;
            }
            return -1;
        }
    }
}

int generation_init_random(Generation* gen,
                           int population_size,
                           int nb_hop,
                           int commune_count) {
    int i, j;
    if (!gen || population_size <= 0 || nb_hop <= 0 || commune_count <= 0) return 1;
    if (nb_hop > commune_count) return 1;

    gen->items = (Individu*)malloc(sizeof(Individu) * population_size);
    if (gen->items == NULL) return 1;

    gen->size = population_size;
    gen->capacity = population_size;
    gen->best_index = -1;

    for (i = 0; i < population_size; i++) {
        int initial_hops = 1 + rand() % nb_hop;
        Individu* ind = &gen->items[i];
        ind->nb_hop = initial_hops;
        ind->fitness = 0.0;
        ind->nb_chru = 0;
        ind->indices = NULL;
        ind->indices = (int*)malloc(sizeof(int) * initial_hops);
        if (!ind->indices) {
            gen->size = i;
            generation_free(gen);
            return 1;
        }
        for (j = 0; j < initial_hops; j++) {
            int cand = rand() % commune_count;
            while (contains_int(ind->indices, j, cand)) cand = rand() % commune_count;
            ind->indices[j] = cand;
        }
    }
    return 0;
}

int generation_init_weighted(Generation* gen,
                             int population_size,
                             int nb_hop,
                             const MunicipalityTable* table) {
    int i, j;
    int commune_count;
    long *cumulative;
    long total;

    if (!gen || !table) return 1;
    if (population_size <= 0 || nb_hop <= 0 || table->count == 0) return 1;

    commune_count = table->count;
    if (nb_hop > commune_count) return 1;

    cumulative = (long *)malloc(sizeof(long) * commune_count);
    if (!cumulative) return 1;

    total = 0;
    for (i = 0; i < commune_count; i++) {
        long w = (long)table->items[i].population;
        if (w < 0) w = 0;
        total += w;
        cumulative[i] = total;
    }

    gen->items = (Individu*)malloc(sizeof(Individu) * population_size);
    if (gen->items == NULL) {
        free(cumulative);
        return 1;
    }

    gen->size = population_size;
    gen->capacity = population_size;
    gen->best_index = -1;

    for (i = 0; i < population_size; i++) {
        int initial_hops = 1 + rand() % nb_hop;
        Individu* ind = &gen->items[i];
        ind->nb_hop = initial_hops;
        ind->fitness = 0.0;
        ind->nb_chru = 0;
        ind->indices = NULL;
        ind->indices = (int*)malloc(sizeof(int) * initial_hops);
        if (!ind->indices) {
            gen->size = i;
            generation_free(gen);
            free(cumulative);
            return 1;
        }
        for (j = 0; j < initial_hops; j++) {
            int cand = pick_unique_weighted(cumulative,
                                            commune_count,
                                            total,
                                            ind->indices,
                                            j);
            if (cand < 0) {
                gen->size = i + 1;
                generation_free(gen);
                free(cumulative);
                return 1;
            }
            ind->indices[j] = cand;
        }
    }

    free(cumulative);
    return 0;
}

void generation_free(Generation* gen) {
    int i;
    if (!gen) return;
    if (gen->items) {
        for (i = 0; i < gen->size; i++) {
            free(gen->items[i].indices);
            gen->items[i].indices = NULL;
        }
        free(gen->items);
        gen->items = NULL;
    }
    gen->size = 0;
    gen->capacity = 0;
    gen->best_index = -1;
}
