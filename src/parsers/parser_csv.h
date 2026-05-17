#ifndef CSV_PARSER_H
#define CSV_PARSER_H

/**
 * @file parser_csv.h
 * @brief Header file for CSV parsing functions related to Municipality data.
 */

#include "../data/municipality.h"

/**
 * @brief Error codes for the CSV parser.
 */
typedef enum
{
    CSV_PARSER_OK = 0,
    CSV_PARSER_ERR_IO = -1,
    CSV_PARSER_ERR_MEMORY = -2,
    CSV_PARSER_ERR_FORMAT = -3,
} CsvParserError;

/**
 * @brief Parses a CSV file into a MunicipalityTable.
 * @param path The path to the CSV file.
 * @param out The table to populate.
 * @return The error code.
 * The caller is responsible for calling municipality_table_free() on success.
 */
CsvParserError csv_parser_parse_file(const char *path, MunicipalityTable *out);

/**
 * @brief Returns a human-readable string for a given CsvParserError code.
 * @param err The error code.
 * @return A string describing the error.
 */
const char *csv_error_mapper(CsvParserError err);

#endif
