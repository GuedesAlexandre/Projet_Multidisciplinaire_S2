#include "municipality.h"
#include <stdlib.h>

void municipality_table_free(MunicipalityTable *table)
{
    if (!table)
        return;
    free(table->items);
    table->items = NULL;
    table->count = 0;
    table->capacity = 0;
}
