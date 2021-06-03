#ifndef _gdb_mysql_h_
#define _gdb_mysql_h_
#include "gdb.h"

#ifdef __cplusplus
extern "C" {
#endif

gdbdrv_t       gdb_mtsqldriver() ;

/* Database connection constructor & destructor: */
pgdb_t         gdb_mysql_new(char *server, int port, char *dbname, char *login, char *passwd);
pgdb_t         gdb_mysql_destroy(pgdb_t db);

/* Errors & errors strings */
gdberrcodes_t  gdb_mysql_error(pgdb_t db);

/* Query execution (constructor & destructor) : */
pquery_t       gdb_mysql_query_new(pgdb_t gdb, char *sql);
pquery_t       gdb_mysql_query_destroy(pquery_t query);

/* Fetch next row: */
int            gdb_mysql_query_row_next(pquery_t query);

/* Get column (value or new allocated pointer to char): */
any_t          gdb_mysql_query_col(pquery_t query, int colnum);
any_t          gdb_mysql_query_col_by_name(pquery_t query, char *name); /* optional */
int            gdb_mysql_query_col_id(pquery_t query, char *name);

/* Other optional stuff: */
size_t         gdb_mysql_query_col_size(pquery_t query, int colnum);
char *         gdb_mysql_query_col_name(pquery_t query, int colnum);
gdb_datatype_t gdb_mysql_query_col_type(pquery_t query, int colnum); 

#ifdef __cplusplus
}
#endif

#endif 
