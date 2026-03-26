#ifndef MUNICIPALITY_H
#define MUNICIPALITY_H

#include <stddef.h>

/* Expected CSV columns (positional, no header row):
 * 0  insee_code    int
 * 1  name          string
 * 2  region_code   int
 * 3  region_name   string
 * 4  dept_code     int
 * 5  dept_name     string
 * 6  postal_code   int
 * 7  population    int
 * 8  latitude      double
 * 9  longitude     double
 */


/** 
 * @struct Municipality
 * @brief Represents a municipality with relevant data fields.
 * Note: Some CSV fields are ignored and not included in this struct.
 */
typedef struct {
    int    insee_code;    
    char   name[64];      
    int    dept_code;     
    char   dept_name[32]; 
    int    population;   
    double latitude;      
    double longitude;    
} Municipality;


/** 
 * @struct MunicipalityTable
 * @brief A dynamic array of Municipality items.
 */
typedef struct {
    Municipality *items;
    size_t   count;
    size_t   capacity;
} MunicipalityTable;

/** 
 * @brief Frees the memory allocated for a MunicipalityTable.
 * @param table The table to free.
 * @return void
 * Note : If the pointer is NULL, the function does nothing.
 * After calling this function, the table's items pointer is set to NULL and count and capacity are reset to 0.
 */
void municipality_table_free(MunicipalityTable *table);

#endif 