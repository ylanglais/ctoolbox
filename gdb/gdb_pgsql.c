#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <libpq-fe.h>
#include <tbx/err.h>


/*
Inclusion of catalog/pgsql_type.h generates a compilation error
#include <catalog/pgsql_type.h>
*/

#include "pgtypes.h"

#define PGNAME 		NAMEOID
#define PGTEXT 		TEXTOID
#define PGCHAR 	  	BPCHAROID
#define PGVARCHAR 	VARCHAROID
#define PGBOOL 		BOOLOID
#define PGINT2		INT2OID
#define PGINT4		INT4OID
#define PGINT8		INT8OID
#define PGFLOAT4	FLOAT4OID
#define PGFLOAT8	FLOAT8OID

#define cp(x, i)  (((char *) (&x))[i])

uint16_t
_gdb_pgsql_bswap16(uint16_t v) {
	uint16_t t;
 	cp(t, 0) = cp(v, 1);
 	cp(t, 1) = cp(v, 0);
	return t;
}

uint32_t
_gdb_pgsql_bswap32(uint32_t v) {
	uint32_t t;
 	cp(t, 0) = cp(v, 3);
 	cp(t, 1) = cp(v, 2);
 	cp(t, 2) = cp(v, 1);
 	cp(t, 3) = cp(v, 0);
	return t;
}

uint64_t
_gdb_pgsql_bswap64(uint64_t v) {
	uint64_t t;
 	cp(t, 0) = cp(v, 7);
 	cp(t, 1) = cp(v, 6);
 	cp(t, 2) = cp(v, 5);
 	cp(t, 3) = cp(v, 4);
 	cp(t, 4) = cp(v, 3);
 	cp(t, 5) = cp(v, 2);
 	cp(t, 6) = cp(v, 1);
 	cp(t, 7) = cp(v, 0);
	return t;
}

#include "gdb.h"
#include "gdb_defs.h"
#include "gdb_pgsql.h"

typedef struct {
	PGconn  *pdb; 
	int 	debug_level;
	gdberrcodes_t last_error;
} gdb_pgsql_t, *pgdb_pgsql_t;

typedef struct {
	gdbdrv_t	drv;
	PGresult    *pres;
	int			currow;
} gdb_pgsql_query_t, *pgdb_pgsql_query_t;

gdbdrv_t
gdb_pgsql_driver() {
	gdbdrv_t drv;
	drv.drv_new               = gdb_pgsql_new;
	drv.drv_destroy           = gdb_pgsql_destroy;
	drv.drv_error             = gdb_pgsql_error;
	drv.drv_query_new         = gdb_pgsql_query_new;
	drv.drv_query_destroy     = gdb_pgsql_query_destroy;
	drv.drv_query_row_next    = gdb_pgsql_query_row_next;
	drv.drv_query_col         = gdb_pgsql_query_col;
	drv.drv_query_col_id      = gdb_pgsql_query_col_id;
	drv.drv_query_col_by_name = gdb_pgsql_query_col_by_name;
	drv.drv_query_col_size    = gdb_pgsql_query_col_size;
	drv.drv_query_col_name    = gdb_pgsql_query_col_name;
	drv.drv_query_col_type    = gdb_pgsql_query_col_type;

	return drv;
}

/* Return last pg error code and reset error code to gdbNOERROR */
gdberrcodes_t
gdb_pgsql_error(pgdb_t db) {
	int r;
	pgdb_pgsql_t gdb_pgsql = (pgdb_pgsql_t) db;
	if (!gdb_pgsql) return gdbNOERROR;
	r = gdb_pgsql->last_error;
	gdb_pgsql->last_error = gdbNOERROR;
	return r;
}

pgdb_t
gdb_pgsql_destroy(pgdb_t db) {
	pgdb_pgsql_t gdb_pgsql = (pgdb_pgsql_t) db;
	
	if (!gdb_pgsql) return 0;
	if (gdb_pgsql->pdb) { 
		PQfinish(gdb_pgsql->pdb); 
		gdb_pgsql->pdb  = NULL; 
	}
	free(gdb_pgsql);
	return NULL;
}

pgdb_t
gdb_pgsql_new(char *server, int port, char *login, char *passwd, char *dbname) {
	pgdb_pgsql_t gdb_pgsql = NULL;
	char sport[10];
	char *dbglvl;

	if (!login || strlen(login) < 2) {
		err_error("Invalid login");
		return NULL;
	}

	if (!(gdb_pgsql = (pgdb_pgsql_t) malloc(sizeof(gdb_pgsql_t)))) {
		err_error("No memory");
		return NULL;
	}

	if ((dbglvl = getenv("PGDBG")) || (dbglvl = getenv("DBDBG"))) {
		switch (*dbglvl) {
		case 1: // err_MESSAGE
			gdb_pgsql->debug_level = err_MESSAGE;	
			break;
		case 2: // err_ERROR
			gdb_pgsql->debug_level = err_ERROR;	
			break;
		case 3: // err_WARN
			gdb_pgsql->debug_level = err_WARNING;	
			break;
		case 4: // err_INFO
			gdb_pgsql->debug_level = err_INFO;	
			break;
		case 5: // err_DEBUG
			gdb_pgsql->debug_level = err_DEBUG;	
			break;
		default: // err_NONE (keep quiet)
			gdb_pgsql->debug_level = err_NONE;	
			break;
		}	
	}
	memset(gdb_pgsql, 0, sizeof(gdb_pgsql_t));

	if (port == 0) port = 5432;
	sprintf(sport, "%d", port);

	gdb_pgsql->pdb = PQsetdbLogin(server, sport, NULL, NULL, dbname, login, passwd);
	if (PQstatus(gdb_pgsql->pdb) == CONNECTION_BAD) {
		err_error("%s", PQerrorMessage(gdb_pgsql->pdb));
		return gdb_pgsql_destroy(gdb_pgsql);	
	}
	return (pgdb_t) gdb_pgsql;
}

pquery_t
gdb_pgsql_query_destroy(pquery_t query) {
	pgdb_pgsql_query_t q = (pgdb_pgsql_query_t) query;
	if (q) { 
		if (q->pres)   { 
			PQclear(q->pres); 
			q->pres = NULL; 
		}
		free(q);
	}
	return NULL;
}

pquery_t 
gdb_pgsql_query_new(pgdb_t db, char *sql) {
	pgdb_pgsql_query_t q;
	pgdb_pgsql_t gdb_pgsql = (pgdb_pgsql_t) db;

	if (!gdb_pgsql) {
		gdb_pgsql->last_error = gdbNOCONNECTION;
		err_error("No connection");
		return NULL;
	}
	if (!(q = (pgdb_pgsql_query_t) malloc(sizeof(gdb_pgsql_query_t)))) {
		gdb_pgsql->last_error = gdbNOMEMORY;
		err_error("No mempry");
		return NULL;
	}

	memset(q, 0, sizeof(gdb_pgsql_query_t));
	q->drv = gdb_pgsql_driver();

	if (!(q->pres = PQexecParams(gdb_pgsql->pdb, sql, 0, NULL, NULL, NULL, NULL, 1))) {
		if (gdb_pgsql->debug_level >= err_ERROR)
			err_error("%s", PQerrorMessage(gdb_pgsql->pdb));
		gdb_pgsql->last_error = gdbCANNOTCREATEQUERY;
		return NULL;
	}
	switch (PQresultStatus(q->pres)) {
	case PGRES_BAD_RESPONSE:
	case PGRES_NONFATAL_ERROR: 
	case PGRES_FATAL_ERROR:
		q->currow = -1;
		err_error("%s", PQerrorMessage(gdb_pgsql->pdb));
		q = gdb_pgsql_query_destroy(q);

	case PGRES_EMPTY_QUERY:
	case PGRES_COMMAND_OK:
	case PGRES_COPY_OUT:
	case PGRES_COPY_IN:
		PQclear(q->pres); 
		q->pres = NULL; 
		break;
	case PGRES_TUPLES_OK:
	default:
		break;
		
	}
	q->currow = -1;
	return (pquery_t) q;
}

int
gdb_pgsql_query_row_next(pquery_t query) {
	pgdb_pgsql_query_t q = (pgdb_pgsql_query_t) query;
	int r;
	if (!q || !q->pres)   return 0;
	r = PQntuples(q->pres);
	if (r < -1) 		  return 0;
	if (r > ++q->currow)  return 1;

	/* no more rows, clear result: */
	PQclear(q->pres); 
	q->pres = NULL; 
	
	return 0;
}

any_t
gdb_pgsql_query_col(pquery_t query, int colnum) {
	pgdb_pgsql_query_t q = (pgdb_pgsql_query_t) query;
	any_t any = { .type = gdbUNDEF, .size = 0, .mustfree = 0 };

	int type;
	char *p;

	int len;

	if (!q || !q->pres) 
		return any;
	if (q->currow < 0 || q->currow >= PQntuples(q->pres)) 
		return any;
	if (colnum < 0 || colnum > PQnfields(q->pres)) 
		return any;

	type = PQftype(q->pres, colnum);
	len  = PQgetlength(q->pres, q->currow, colnum);
	
	if (len < 0) return any;

	p = PQgetvalue(q->pres, q->currow, colnum);

	switch (type) {
	case PGNAME:
	case PGTEXT:
	case PGCHAR:
	case PGVARCHAR:
		any.mustfree    = 1;
		any.type        = gdbSTRING;
		any.size        = PQgetlength(q->pres, q->currow, colnum); //strlen(p) * sizeof(char);
		any.val._string = strdup(p);
		break;

	case PGBOOL:
		any.mustfree    = 0;
		any.type        = gdbBOOL;
		any.size        = sizeof(char);
		any.val._bool   = *p;
		break;
	
	case PGINT2:
		any.mustfree    = 0;
		any.type        = gdbINT2;
		any.size        = sizeof(short);
		any.val._int2   = _gdb_pgsql_bswap16(*(uint16_t *) p);
		break;

	case PGINT4:
		any.mustfree    = 0;
		any.type        = gdbINT4;
		any.size        = sizeof(int);
		any.val._int4   = _gdb_pgsql_bswap32(*(uint32_t *) p);
		break;

	case PGINT8:
		any.mustfree    = 0;
		any.type        = gdbINT8;
		any.size        = sizeof(long long);
		any.val._int8   = _gdb_pgsql_bswap64(*(uint64_t *) p);
		break;

	case PGFLOAT4:
		any.mustfree    = 0;
		any.type        = gdbFLOAT4;
		any.size        = sizeof(float);
		any.val._int4   = _gdb_pgsql_bswap32(*(uint32_t *) p);
		break;
	
	case PGFLOAT8:
		any.mustfree    = 0;
		any.type        = gdbFLOAT8;
		any.size        = sizeof(double);
		any.val._int8   = _gdb_pgsql_bswap64(*(uint64_t *) p);
		break;
		
	default:
		err_error(">>> column %d: unsopported datatype = %d returned as STRING", colnum, type);
		any.mustfree    = 1;
		any.type        = gdbUNDEF;
		any.size        = PQgetlength(q->pres, q->currow, colnum); //strlen(p) * sizeof(char);
		any.val._string = strdup(p);
		break;
	}

	return any;
}

int
gdb_pgsql_query_col_id(pquery_t query, char *name) {
	pgdb_pgsql_query_t q = (pgdb_pgsql_query_t) query;
	int i, n;
	if (!q || !q->pres )            return -1;
	if (!name || strlen(name) < 2)  return -2;

	n = PQnfields(q->pres);

	for (i = 0; i < n; i++) {
		if (!strcmp(name, PQfname(q->pres, i)))	
			return i;
	}
	return -3;
}

any_t
gdb_pgsql_query_col_by_name(pquery_t query, char *name) {
	pgdb_pgsql_query_t q = (pgdb_pgsql_query_t) query;
	any_t any = { .type = gdbUNDEF, .size = 0, .mustfree = 0 };
	int i;
	if (!q || !q->pres) 
		return any;

	if (q->currow < 0 || q->currow >= PQntuples(q->pres)) 
		return any;

	if (!name || strlen(name) < 2) return any;

	if ((i = gdb_pgsql_query_col_id(q, name)) > -1) 
		return gdb_pgsql_query_col(q, i);

	return any;
}

size_t
gdb_pgsql_query_col_size(pquery_t query, int colnum) {
	pgdb_pgsql_query_t q = (pgdb_pgsql_query_t) query;
	if (!q || !q->pres) return 0;
	if (colnum < 0 || colnum >= PQnfields(q->pres)) return 0;
	int i;
	if ((i = PQfsize(q->pres, colnum)) < 0) {
		return PQfmod(q->pres, colnum);
	}
	return i;
}

char *
gdb_pgsql_query_col_name(pquery_t query, int colnum) {
	pgdb_pgsql_query_t q = (pgdb_pgsql_query_t) query;
	if (!q) return NULL;
	if (colnum < 0 || colnum >= PQnfields(q->pres)) return NULL;
	return strdup(PQfname(q->pres, colnum));
}

gdb_datatype_t
gdb_pgsql_query_col_type(pquery_t query, int colnum) {
	pgdb_pgsql_query_t q = (pgdb_pgsql_query_t) query;
	if (!q) return gdbUNDEF;
	if (colnum < 0 || colnum >= PQnfields(q->pres)) return gdbUNDEF;

	switch (PQftype(q->pres, colnum)) {
	case PGNAME:
	case PGTEXT:
	case PGCHAR:
	case PGVARCHAR:
		return gdbSTRING;
	case PGBOOL:
		return gdbBOOL;	
	case PGINT2:
		return gdbINT2;
	case PGINT4:
		return gdbINT4;
	case PGINT8:
		return gdbINT8;
	case PGFLOAT4:
		return gdbFLOAT4;
	case PGFLOAT8:
		return gdbFLOAT8;
	default:
		break;
	}
	return gdbUNDEF;
}

#ifdef _test_gdb_pgsql_

#include <stdio.h> 

#include "gdb_test.conf"

static char * gdb_type_names[] = {
	"dbUNDEF",  // 0
	"dbBOOL",   // 1
	"dbSTRING", // 2
	"dbINT2",   // 3
	"dbINT4",   // 4
	"dbINT8",   // 5
	"dbFLOAT4", // 6
	"dbFLOAT8"  // 7
};

int
main(int n, char *a[]) {
	pgdb_t db;
	pquery_t q;

	if (!(db = gdb_pgsql_new(dbserver, dbport, dblogin, dbpasswd, dbname))) {
		err_error("Cannot connect to DB");
		exit(1);
	}
	if (!(q  = gdb_pgsql_query_new(db, "select * from t1"))) {
		err_error("Cannot create query");	
		gdb_pgsql_destroy(db);
		exit(2);
	}
	
	int r = 0;
	while (gdb_pgsql_query_row_next(q)) {
		if (r == 0) {
			int i;
			printf("Columns are:\n");
			for (i = 0; i <= 6; i++) {
				char *cn = NULL;
				printf("%d: name = %s type = %-10s (%d) size = %2d\n", 
					i,
					cn = gdb_pgsql_query_col_name(q, i),
					gdb_type_names[gdb_pgsql_query_col_type(q, i)],
					gdb_pgsql_query_col_type(q, i),
					gdb_pgsql_query_col_size(q, i));
				if (cn) {
					free(cn);
					cn = NULL;
				}
			}	
			any_t a;
			a = gdb_pgsql_query_col_by_name(q, "name");
			printf("\nquery_col_by_name(q, \"name\") = \"%s\"\n", a.val._string);
			if (a.val._string) free(a.val._string);
			printf("\nDump t1 table:\n");
			r++;
		}
		any_t res[7];
		int i;
		for (i = 0; i < 7; i++) {
			res[i] = gdb_pgsql_query_col(q, i);
		}
		
		printf("%s;%d;%d;%d;%d;%f;%lf\n",
			res[0].val._string,
			res[1].val._bool,
			res[2].val._int2,
			res[3].val._int4,
			res[4].val._int8,
			res[5].val._float4,
			res[6].val._float8);
		for (i = 0; i  < 7; i++) {
			if (res[i].mustfree && res[i].val._string) free(res[i].val._string);
		}
	}
	gdb_pgsql_query_destroy(q);
	gdb_pgsql_destroy(db);
		
	return 0;
}
#endif
