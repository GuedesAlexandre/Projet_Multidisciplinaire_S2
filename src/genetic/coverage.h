#ifndef COVERAGE_H
#define COVERAGE_H

#include "../data/municipality.h"

/**
 * @brief Coverage index: for each commune, a list of communes within radius.
 */
typedef struct {
    int count;
    int *offsets; /* size count + 1 */
    int *indices; /* flattened lists */
} CoverageIndex;

/**
 * @brief Builds the coverage index using haversine distance and radius.
 * @return 0 on success, 1 on error.
 */
int coverage_build(const MunicipalityTable *table, CoverageIndex *out_index);

/**
 * @brief Frees the coverage index.
 */
void coverage_free(CoverageIndex *index);

#endif /* COVERAGE_H */
