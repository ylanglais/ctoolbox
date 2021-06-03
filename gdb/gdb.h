#ifndef _gdb_h_
#define _gdb_h_

typedef enum { gdbPOSTGRES, gdbMYSQL, gdbODBC /*, gdbSYBASE*/ , gdbUNKNOWN } gdbtype_t;

typedef union {
	char       _bool;
	char      *_string;
	short	   _int2;
	int 	   _int4;
	long long  _int8;
	float	   _float4;
	double	   _float8;
} any_u, *pany_u;

typedef enum {
	gdbUNDEF   = 0,
	gdbBOOL    = 1,
	gdbSTRING  = 2,
	gdbINT2    = 3,
	gdbINT4    = 4,
	gdbINT8    = 5,
	gdbFLOAT4  = 6,
	gdbFLOAT8  = 7,
	gdbLAST
} gdb_datatype_t;

typedef struct {
	gdb_datatype_t type; 
	size_t	       size;
	any_u	       val;
	int		       mustfree;
} any_t; //= { .type = gdbUNDEF, .size = 0, .mustfree = 0 };

typedef enum {
	gdbNOERROR,
	gdbCANNOTLOADDRIVER,
	gdbCANNOTRESOLV,
	gdbNOCONNECTION,
	gdbALREADYCONNECTED,
	gdbNOMEMORY,
	gdbLOGININVALID,
	gdbPASSWDINVALID,
	gdbLOGPASINCORRECT,

	gdbODBCCANNOTALLOCATEENVHANDLE,
	gdbODBCCANNOTSETENV,
	gdbODBCCANNOTALLOCATESQLHANDLE,

	gdbCANNOTCONNECT,
	gdbCANNOTCREATEQUERY,
	gdbBADSTATEMENT,
	gdbNOMORE
} gdberrcodes_t;

#define gdbOK gdbNOERROR

#ifndef _gdb_c_
typedef void *pgdb_t;
typedef void *pquery_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

char * 		   gdb_error_string(gdberrcodes_t i);

char *	       gdb_type_to_name(gdbtype_t type);
gdbtype_t 	   gdb_name_to_type(char *name);

pgdb_t         gdb_new(gdbtype_t engine, char *server, int port, char *login, char *passwd, char *gdbname);
pgdb_t         gdb_destroy(pgdb_t gdb);


/* Return last gdbsql error code and reset error code to gdbNOERROR */
int            gdb_error(pgdb_t gdb);
char *         gdb_error_string(gdberrcodes_t errcode);

/* Query execution & free: */
pquery_t       gdb_query_new(pgdb_t gdb, char *sql);
pquery_t       gdb_query_destroy(pquery_t q);

/* Fetch next row: */
int            gdb_query_row_next(pquery_t q);

/* Get column (value or new allocated pointer to char): */
any_t          gdb_query_col(pquery_t q, int colnum);
any_t          gdb_query_col_by_name(pquery_t q, char *name); /* optional */

/* Free data if required: */
void		   gdb_any_dispose(any_t any);

/* Other optional stuff: */
size_t         gdb_query_col_size(pquery_t q, int colnum);
char *         gdb_query_col_name(pquery_t q, int colnum); /* MUST FREE returned name */
gdb_datatype_t gdb_query_col_type(pquery_t q, int colnum); 

#ifdef __cplusplus
}
#endif

#endif /* _gdb_h_ */

