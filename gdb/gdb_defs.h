#ifndef _gdb_defs_
#define _gdb_defs_

typedef pgdb_t        (*new_f)(char *, int, char *, char *, char *);
typedef pgdb_t        (*destroy_f)(pgdb_t gdb);
typedef gdberrcodes_t (*error_f)(pgdb_t);
typedef pquery_t      (*query_new_f)(pgdb_t, char *);
typedef pquery_t      (*query_destroy_f)(pquery_t);
typedef int           (*query_row_next_f)(pquery_t);
typedef any_t         (*query_col_f)(pquery_t, int);
typedef int           (*query_col_id_f)(pquery_t, char *);
typedef any_t         (*query_col_by_name_f)(pquery_t, char *);
typedef size_t        (*query_col_size_f)(pquery_t, int);
typedef char *        (*query_col_name_f)(pquery_t, int);
typedef gdb_datatype_t (*query_col_type_f)(pquery_t, int); 


typedef struct _gdbdrv_t {
	new_f				drv_new;
	destroy_f 			drv_destroy;
	error_f				drv_error;
	query_new_f			drv_query_new;
	query_destroy_f 	drv_query_destroy;
	query_row_next_f 	drv_query_row_next;
	query_col_f		 	drv_query_col;
    query_col_id_f		drv_query_col_id;
    query_col_by_name_f drv_query_col_by_name;
    query_col_size_f	drv_query_col_size;
    query_col_name_f	drv_query_col_name;
	query_col_type_f	drv_query_col_type;
} gdbdrv_t, *pgdbdrv_t;

gdbdrv_t  gdb_driver(pgdb_t gdb);

#endif
