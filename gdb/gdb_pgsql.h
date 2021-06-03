#ifndef _gdb_pgsql_h_
#define _gdb_pgsql_h_

#include "gdb.h"

#ifdef __cplusplus
extern "C" {
#endif

gdbdrv_t       gdb_pgsqldriver() ;

/* Database connection constructor & destructor: */
pgdb_t         gdb_pgsql_new(char *server, int port, char *dbname, char *login, char *passwd);
pgdb_t         gdb_pgsql_destroy(pgdb_t db);

/* Errors & err strings: */
gdberrcodes_t  gdb_pgsql_error(pgdb_t db);

/* execute a statement: */
pquery_t       gdb_pgsql_query_new(pgdb_t gdb, char *sql);
pquery_t       gdb_pgsql_query_destroy(pquery_t query);

/* fetch next row: */
int            gdb_pgsql_query_row_next(pgdb_t pg);

/* Get column (value or new allocated pointer to char): */
any_t          gdb_pgsql_query_col(pgdb_t pg, int colnum);
any_t          gdb_pgsql_query_col_by_name(pgdb_t pg, char *name); /* optional */
int            gdb_pgsql_query_col_id(pquery_t query, char *name);

/* other optional stuff: */
size_t         gdb_pgsql_query_col_size(pquery_t query, int colnum);
char *         gdb_pgsql_query_col_name(pquery_t query, int colnum);
gdb_datatype_t gdb_pgsql_query_col_type(pquery_t query, int colnum); 

#ifdef __cplusplus
}
#endif
#endif 
