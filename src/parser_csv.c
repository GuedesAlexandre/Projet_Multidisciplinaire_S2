#include "parser_csv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @file parser_csv.c
 * @brief Implementation of CSV parsing functions for Municipality data.
 * The parser reads a CSV file, extracts relevant fields, and populates a MunicipalityTable.
 */

#define LINE_BUF 256
#define INITIAL_CAP 512
#define NUM_FIELDS 10

/* Expected CSV columns (positional, no header row):
 * 0  insee_code   int     <- kept
 * 1  name         string  <- kept
 * 2  region_code  int     <- ignored
 * 3  region_name  string  <- ignored
 * 4  dept_code    int     <- kept
 * 5  dept_name    string  <- kept
 * 6  postal_code  int     <- ignored
 * 7  population   int     <- kept
 * 8  latitude     double  <- kept
 * 9  longitude    double  <- kept
 */

/**
 * @brief Splits a line into fields based on commas.
 * @param line The line to split.
 * @param fields An array to store the field pointers.
 * @param max_fields The maximum number of fields to parse.
 * @return The number of fields found.
 */
static int split_line(char *line, char *fields[], int max_fields)
{
    int count = 0;
    char *p = line;

    while (count < max_fields)
    {
        fields[count++] = p;
        p = strchr(p, ',');
        if (!p)
            break;
        *p = '\0';
        p++;
    }
    return count;
}

/**
 * @brief Safely copies a string, ensuring null termination.
 * @param dst The destination buffer.
 * @param src The source string.
 * @param dst_size The size of the destination buffer.
 */
static void safe_copy(char *dst, const char *src, size_t dst_size)
{
    strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = '\0';
}

/**
 * @brief Parses a single row of CSV data into a Municipality struct.
 * @param fields An array of field pointers.
 * @param n The number of fields.
 * @param out The Municipality struct to populate.
 * @return CSV_PARSER_OK on success, CSV_PARSER_ERR_FORMAT on format error.
 */
static CsvParserError parse_row(char *fields[], size_t n, Municipality *out)
{
    if (n < NUM_FIELDS)
        return CSV_PARSER_ERR_FORMAT;
    out->insee_code = atoi(fields[0]);
    safe_copy(out->name, fields[1], sizeof(out->name));
    out->dept_code = atoi(fields[4]);
    safe_copy(out->dept_name, fields[5], sizeof(out->dept_name));
    out->population = atoi(fields[7]);
    out->latitude = atof(fields[8]);
    out->longitude = atof(fields[9]);
    return CSV_PARSER_OK;
}

/**
 * @brief Grows the capacity of a MunicipalityTable.
 * @param t The table to grow.
 * @return 0 on success, -1 on failure.
 * @note The function doubles the capacity when growing, starting from INITIAL_CAP if the current capacity is 0, otherwise it multiplies the current capacity by 2.
 * If memory allocation fails, the function returns -1 and the table remains unchanged.
 */
static int table_grow(MunicipalityTable *t)
{
    size_t new_cap = t->capacity ? t->capacity * 2 : INITIAL_CAP;
    Municipality *new_items = realloc(t->items, new_cap * sizeof(Municipality));
    if (!new_items)
        return -1;
    t->items = new_items;
    t->capacity = new_cap;
    return 0;
}

/**
 * @brief Parses a CSV file into a MunicipalityTable.
 * @param path The path to the CSV file.
 * @param out The table of municipalities to populate.
 * @return The error code.
 * @note Lines with invalid format are skipped, but the function continues parsing the rest of the file.
 * The caller is responsible for calling municipality_table_free() on success.
 */
CsvParserError csv_parser_parse_file(const char *path, MunicipalityTable *out)
{
    FILE *fp = fopen(path, "r");
    if (!fp)
        return CSV_PARSER_ERR_IO;

    out->items = NULL;
    out->count = 0;
    out->capacity = 0;

    char line[LINE_BUF];
    char *fields[NUM_FIELDS];
    CsvParserError err = CSV_PARSER_OK;
    int lineno = 0;

    while (fgets(line, sizeof(line), fp))
    {
        lineno++;
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = '\0';
        if (len == 0)
            continue;
        if (out->count >= out->capacity)
        {
            if (table_grow(out) != 0)
            {
                fprintf(stderr, "line %d: memory allocation failed\n", lineno);
                err = CSV_PARSER_ERR_MEMORY;
                break;
            }
        }
        int fields_count = split_line(line, fields, NUM_FIELDS);
        Municipality row = {0};
        CsvParserError row_err = parse_row(fields, (size_t)fields_count, &row);

        if (row_err != CSV_PARSER_OK)
        {
            fprintf(stderr, "line %d: skipped — invalid format (%d fields)\n",
                    lineno, fields_count);
            continue;
        }
        out->items[out->count++] = row;
    }
   if (err == CSV_PARSER_OK && ferror(fp))
        err = CSV_PARSER_ERR_IO;

    fclose(fp);
    if (err != CSV_PARSER_OK)
        municipality_table_free(out);
    return err;
}

const char *csv_error_mapper(CsvParserError err)
{
    switch (err)
    {
    case CSV_PARSER_OK:
        return "success";
    case CSV_PARSER_ERR_IO:
        return "file I/O error";
    case CSV_PARSER_ERR_MEMORY:
        return "memory allocation failed";
    case CSV_PARSER_ERR_FORMAT:
        return "invalid CSV format";
    default:
        return "unknown error";
    }
}
