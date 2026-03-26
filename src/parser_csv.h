#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include "municipality.h"


typedef enum {
    CSV_PARSER_OK         =  0,
    CSV_PARSER_ERR_IO     = -1,
    CSV_PARSER_ERR_MEMORY = -2,
    CSV_PARSER_ERR_FORMAT = -3,
} CsvParserError;

/* Parse a CSV file into a MunicipalityTable.
 * The caller is responsible for calling municipality_table_free() on success. */
CsvParserError csv_parser_parse_file(const char *path, MunicipalityTable *out);

const char *csv_parser_strerror(CsvParserError err);

#endif 