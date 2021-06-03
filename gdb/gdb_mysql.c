#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <bits/byteswap.h>

#include <mysql/mysql.h>


/*
	MYSQL_TYPE_VARCHAR
	MYSQL_TYPE_VAR_STRING
	MYSQL_TYPE_STRING

	MYSQL_TYPE_SHORT,  
	MYSQL_TYPE_LONG,
	MYSQL_TYPE_LONGLONG,
	MYSQL_TYPE_FLOAT,  
	MYSQL_TYPE_DOUBLE,

	MYSQL_TYPE_NULL,   
	MYSQL_TYPE_TIMESTAMP,
	MYSQL_TYPE_INT24,
	MYSQL_TYPE_DATE,   
	MYSQL_TYPE_TIME,
	MYSQL_TYPE_DATETIME, 
	MYSQL_TYPE_YEAR,
	MYSQL_TYPE_NEWDATE, 
	MYSQL_TYPE_BIT,
	MYSQL_TYPE_NEWDECIMAL=246,
	MYSQL_TYPE_ENUM=247,
	MYSQL_TYPE_SET=248,
	MYSQL_TYPE_TINY_BLOB=249,
	MYSQL_TYPE_MEDIUM_BLOB=250,
	MYSQL_TYPE_LONG_BLOB=251,
	MYSQL_TYPE_BLOB=252,
	MYSQL_TYPE_GEOMETRY=255
*/

#define MYSQLNAME 		MYSQL_TYPE_STRING
#define MYSQLTEXT 		MYSQL_TYPE_VAR_STRING
#define MYSQLCHAR		MYSQL_TYPE_VARCHAR
#define MYSQLVARCHAR 	MYSQL_TYPE_VARCHAR
#define MYSQLBOOL 		MYSQL_TYPE_TINY
#define MYSQLINT2		MYSQL_TYPE_SHORT
#define MYSQLINT4		MYSQL_TYPE_LONG
#define MYSQLINT8		MYSQL_TYPE_LONGLONG
#define MYSQLFLOAT4		MYSQL_TYPE_FLOAT
#define MYSQLFLOAT8		MYSQL_TYPE_DOUBLE

#include <tbx/err.h>

#include "gdb.h"
#include "gdb_defs.h"
#include "gdb_mysql.h"

typedef struct {
	MYSQL  *conn; 
	int 	debug_level;
	gdberrcodes_t  last_error;
} gdb_mysql_t, *pgdb_mysql_t;

typedef struct {
	gdbdrv_t	   drv;
	MYSQL_RES     *pres;
	MYSQL_ROW      row;
	int 	      *flds;
	int		       nf;
	int		       currow;
} gdb_mysql_query_t, *pgdb_mysql_query_t;

gdbdrv_t
gdb_mysql_driver() {
	gdbdrv_t drv;
	drv.drv_new               = gdb_mysql_new;
	drv.drv_destroy           = gdb_mysql_destroy;
	drv.drv_error             = gdb_mysql_error;
	drv.drv_query_new         = gdb_mysql_query_new;
	drv.drv_query_destroy     = gdb_mysql_query_destroy;
	drv.drv_query_row_next    = gdb_mysql_query_row_next;
	drv.drv_query_col         = gdb_mysql_query_col;
	drv.drv_query_col_id      = gdb_mysql_query_col_id;
	drv.drv_query_col_by_name = gdb_mysql_query_col_by_name;
	drv.drv_query_col_size    = gdb_mysql_query_col_size;
	drv.drv_query_col_name    = gdb_mysql_query_col_name;
	drv.drv_query_col_type    = gdb_mysql_query_col_type;

	return drv;
}

/* Return last mysql error code and reset error code to gdbNOERROR */
gdberrcodes_t
gdb_mysql_error(pgdb_t db) {
	pgdb_mysql_t gdb_mysql = (pgdb_mysql_t) db;
	if (!gdb_mysql) return gdbNOERROR;
	int r;
	r = gdb_mysql->last_error;
	gdb_mysql->last_error = gdbNOERROR;
	return r;
}

pgdb_t
gdb_mysql_destroy(pgdb_t db) {
	pgdb_mysql_t gdb_mysql = (pgdb_mysql_t) db;
	if (!gdb_mysql) return NULL;
	if (gdb_mysql->conn) { 
		mysql_close(gdb_mysql->conn); 
		gdb_mysql->conn  = NULL; 
		mysql_library_end();
	}
	free(gdb_mysql);
	return NULL;
}

pgdb_t
gdb_mysql_new(char *server, int port, char *login, char *passwd, char *dbname) {
	char *dbglvl;
	pgdb_mysql_t gdb_mysql = NULL;

	if (!login || strlen(login) < 2) {
		err_error("Invalid login");
		return NULL;
	}

	if (!(gdb_mysql = (pgdb_mysql_t) malloc(sizeof(gdb_mysql_t)))) {
		err_error("No memory");
		return NULL;
	}

	if ((dbglvl = getenv("MYSQLDBG")) || (dbglvl = getenv("DBDBG"))) {
		int dlvl = atoi(dbglvl);
		switch (dlvl) {
		case 1: // err_MESSAGE
			gdb_mysql->debug_level = err_MESSAGE;	
			break;
		case 2: // err_ERROR
			gdb_mysql->debug_level = err_ERROR;	
			break;
		case 3: // err_WARN
			gdb_mysql->debug_level = err_WARNING;	
			break;
		case 4: // err_INFO
			gdb_mysql->debug_level = err_INFO;	
			break;
		case 5: // err_DEBUG
			gdb_mysql->debug_level = err_DEBUG;	
			break;
		default: // err_NONE (keep quiet)
			gdb_mysql->debug_level = err_NONE;	
			break;
		}	
	}
	memset(gdb_mysql, 0, sizeof(gdb_mysql_t));

	/* Connect to database */
	gdb_mysql->conn = mysql_init(NULL);

	/* mysql_real_connect: 
		MYSQL *mysql, 
		char *host, 
		char *user, 
		char *passwd, 
		char *db, 
		unsigned int port, 
		char *unix_socket, 
		unsigned long client_flag)
	*/

	if (!mysql_real_connect(gdb_mysql->conn, server, login, passwd, dbname, port, NULL, 0)) {
		err_error("%s", mysql_error(gdb_mysql->conn));
		gdb_mysql = gdb_mysql_destroy(gdb_mysql);	
		return NULL;
	}

	pquery_t q;

	/* Attempt to make MySQL behave according to ANSI: */
	if ((q = gdb_mysql_query_new(gdb_mysql, "SET sql_mode = 'ansi'"))) {
		gdb_mysql_query_destroy(q);
	}

	return (pgdb_t) gdb_mysql;
}

pquery_t
gdb_mysql_query_destroy(pquery_t query) {
	pgdb_mysql_query_t q = (pgdb_mysql_query_t) query;
	if (q) { 
		if (q->pres)   { 
			mysql_free_result(q->pres); 
			q->pres = NULL; 
		}
		if (q->flds) {
			free(q->flds);
			q->flds = NULL;
		}
		free(q);
	}
	return NULL;
}

pquery_t 
gdb_mysql_query_new(pgdb_t db, char *sql) {
	pgdb_mysql_t gdb_mysql = (pgdb_mysql_t) db;

	pgdb_mysql_query_t q;

	if (!gdb_mysql) {
		err_error("No connection");
		return NULL;
	}
	if (!(q = (pgdb_mysql_query_t) malloc(sizeof(gdb_mysql_query_t)))) {
		gdb_mysql->last_error = gdbNOMEMORY;
		err_error("No memory");
		return NULL;
	}

	memset(q, 0, sizeof(gdb_mysql_query_t));

	q->drv = gdb_mysql_driver();

	if (mysql_query(gdb_mysql->conn, sql)) {
		gdb_mysql->last_error = gdbBADSTATEMENT;
		err_error("%s", mysql_error(gdb_mysql->conn));
		return NULL;
	}

	if (gdb_mysql->debug_level >= err_DEBUG) err_debug("%s", sql);
	
	if (!(q->pres = mysql_store_result(gdb_mysql->conn))) {
		return q;
	}
	q->currow = -1;
	
	return (pquery_t) q;
}

int
gdb_mysql_query_row_next(pquery_t query) {
	pgdb_mysql_query_t q = (pgdb_mysql_query_t) query;
	if (!q || !q->pres)  return 0;

	if (!(q->row = mysql_fetch_row(q->pres))) {
		mysql_free_result(q->pres);
		q->pres = NULL;
		return 0;
	}
	q->nf = mysql_num_fields(q->pres);
	++q->currow;
	return 1;
}

any_t
gdb_mysql_query_col(pquery_t query, int colnum) {
	pgdb_mysql_query_t q = (pgdb_mysql_query_t) query;
	any_t any = { .type = gdbUNDEF, .size = 0, .mustfree = 0 };
	MYSQL_FIELD *fld;
	char *p;

	if (!q || !q->pres)               return any;
	if (!q->row)                      return any;
	if (colnum < 0 || colnum > q->nf) return any;

	p = q->row[colnum];

	fld = mysql_fetch_field_direct(q->pres, colnum);

	#if 0
	if (gdb_mysql->debug_level >= err_TRACE) { 
		err_trace("process column %d: type = %-12s, value = %s", colnum, db_type_names[gdb_mysql_query_col_type(q, colnum)], p);
	}
	#endif

	switch (fld->type) {
	case MYSQLNAME:
	case MYSQLTEXT:
	case MYSQLVARCHAR:
		any.mustfree = 1;
		any.type     = gdbSTRING;
		any.size     = strlen(p) * sizeof(char);
		any.val._string = strdup(p);
		break;

	case MYSQLBOOL:
		any.mustfree = 0;
		any.type     = gdbBOOL;
		any.size     = sizeof(char);
		any.val._bool= *p - '0';
		break;
 
	case MYSQLINT2:
		any.mustfree = 0;
		any.type     = gdbINT2;
		any.size     = sizeof(short);
		sscanf(p, "%hd", &(any.val._int2));
		break;
	case MYSQLINT4:
		any.mustfree = 0;
		any.type     = gdbINT4;
		any.size     = sizeof(int);
		sscanf(p, "%d", &(any.val._int4));
		break;
	case MYSQLINT8:
		any.mustfree = 0;
		any.type     = gdbINT8;
		any.size     = sizeof(long long);
		sscanf(p, "%Ld", &(any.val._int8));
		break;
	case MYSQLFLOAT4:
		any.mustfree = 0;
		any.type     = gdbFLOAT4;
		any.size     = sizeof(float);
		sscanf(p, "%f", &(any.val._float4));
		break;
	case MYSQLFLOAT8:
		any.mustfree = 0;
		any.type     = gdbFLOAT8;
		any.size     = sizeof(double);
		sscanf(p, "%lf", &(any.val._float8));
		break;

	default:
		err_error(">>> column %d: unsopported datatype = %d returned as STRING", colnum, fld->type);
		any.mustfree = 1;
		any.type     = gdbUNDEF;
		any.size     = strlen(p) * sizeof(char);
		any.val._string  = strdup(p);
		break;
	}
	return any;
}

int
gdb_mysql_query_col_id(pquery_t query, char *name) {
	pgdb_mysql_query_t q = (pgdb_mysql_query_t) query;
	int i;
	MYSQL_FIELD *fld;
	if (!q || !q->pres || !q->nf)      return -1;
	if (!name || strlen(name) < 2)     return -2;

	for (i = 0; i < q->nf; i++) {
		fld = mysql_fetch_field_direct(q->pres, i);
		if (!strcmp(name, fld->name))
			return i;
	}
	return -3;
}

any_t
gdb_mysql_query_col_by_name(pquery_t query, char *name) {
	pgdb_mysql_query_t q = (pgdb_mysql_query_t) query;
	any_t any = { .type = gdbUNDEF, .size = 0, .mustfree = 0 };
	int i;
	if (!q || !q->pres || !q->nf)      return any;
	if (!name || strlen(name) < 2)     return any;

	if ((i = gdb_mysql_query_col_id(q, name)) > -1) 
		return gdb_mysql_query_col(q, i);
	
	return any;
}

size_t
gdb_mysql_query_col_size(pquery_t query, int colnum) {
	pgdb_mysql_query_t q = (pgdb_mysql_query_t) query;
	MYSQL_FIELD *fld;
	if (!q || !q->nf) 				   return 0;
	if (colnum < 0 || colnum >= q->nf) return 0;
	fld = mysql_fetch_field_direct(q->pres, colnum);
	return fld->length;
}

char *
gdb_mysql_query_col_name(pquery_t query, int colnum) {
	pgdb_mysql_query_t q = (pgdb_mysql_query_t) query;
	MYSQL_FIELD *fld;
	if (!q || !q->nf) 				   return NULL;
	if (colnum < 0 || colnum >= q->nf) return NULL;
	fld = mysql_fetch_field_direct(q->pres, colnum);
	return strdup(fld->name);
}

gdb_datatype_t
gdb_mysql_query_col_type(pquery_t query, int colnum) {
	pgdb_mysql_query_t q = (pgdb_mysql_query_t) query;
	MYSQL_FIELD *fld;
	if (!q) return gdbUNDEF;

	if (colnum < 0 || colnum >= q->nf) return gdbUNDEF;
	fld = mysql_fetch_field_direct(q->pres, colnum);

	switch (fld->type) {
	case MYSQLNAME:
	case MYSQLTEXT:
	case MYSQLVARCHAR:
		return gdbSTRING;
	case MYSQLBOOL:
		return gdbBOOL;	
	case MYSQLINT2:
		return gdbINT2;
	case MYSQLINT4:
		return gdbINT4;
	case MYSQLINT8:
		return gdbINT8;
	case MYSQLFLOAT4:
		return gdbFLOAT4;
	case MYSQLFLOAT8:
		return gdbFLOAT8;
	default:
		err_error("unsupported type = %d\n", fld->type);
		break;
	}
	return gdbUNDEF;
}
 
#ifdef _test_gdb_mysql_

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

	if (!(db = gdb_mysql_new(dbserver, dbport, dblogin, dbpasswd, dbname))) {
		err_error("Cannot connect to DB");
		exit(1);
	}
	if (!(q  = gdb_mysql_query_new(db, "select * from t1"))) {
		err_error("Cannot create query");	
		gdb_mysql_destroy(db);
		exit(2);
	}
	
	int r = 0;
	while (gdb_mysql_query_row_next(q)) {
		if (r == 0) {
			int i;
			printf("Columns are:\n");
			for (i = 0; i <= 6; i++) {
				char *cn;
				printf("%d: name = %s type = %-10s (%d) size = %2d\n", 
					i,
					cn = gdb_mysql_query_col_name(q, i),
					gdb_type_names[gdb_mysql_query_col_type(q, i)],
					gdb_mysql_query_col_type(q, i),
					gdb_mysql_query_col_size(q, i));
				if (cn) {
					free(cn); 
					cn = NULL;
				}
			}	
			any_t a;
			a = gdb_mysql_query_col_by_name(q, "name");
			printf("\nquery_col_by_name(q, \"name\") = \"%s\"\n", a.val._string);
			if (a.val._string) free(a.val._string);
			printf("\nDump t1 table:\n");
			r++;
		}
		any_t res[7];
		int i;
		for (i = 0; i < 7; i++) {
			res[i] = gdb_mysql_query_col(q, i);
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
	gdb_mysql_query_destroy(q);
	gdb_mysql_destroy(db);
		
	return 0;
}

#endif
